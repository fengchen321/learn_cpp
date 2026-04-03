#pragma once

#include <cstddef>
#include <cstdint>

#if defined(PERF_ENABLED)
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <type_traits>
#include <vector>
#endif

namespace perf_duration_trace {

struct SiteDesc {
    const char* label;
    const char* file;
    const char* function;
    uint32_t line;
};

struct SiteRef {
    uint32_t id;
    SiteDesc site_desc;
};

struct Token {
    // Token is an ordinary value type. If it crosses threads, the caller must
    // publish and consume it through synchronization that creates a happens-before edge.
    uint64_t start_ns = 0;
    uint32_t site_id = 0;

    [[nodiscard]] bool valid() const noexcept { return start_ns != 0U; }
};

struct SampleRecord {
    uint64_t start_ns = 0;
    uint64_t duration_ns = 0;
    uint32_t site_id = 0;
    uint32_t tid_hash = 0;
    uint32_t seq_no = 0;
    uint16_t cpu_hint = 0;
    uint16_t flags = 0;
};

struct ExportPaths {
    const char* sample_path = "perf_duration_trace.perfbin";
    const char* site_path = "perf_duration_trace.sites.tsv";
};

struct AsyncExportConfig {
    ExportPaths paths {};
    uint32_t flush_interval_ms = 100;
};

struct Config {
    size_t shard_count = 0;
    size_t capacity_per_shard = 4096;
};

struct ExportStats {
    uint64_t exported_samples = 0;
    uint64_t dropped_samples = 0;
    uint64_t registered_sites = 0;
    uint32_t shard_count = 0;
    uint32_t capacity_per_shard = 0;
    bool success = false;
};

class Runtime final {
 public:
    [[nodiscard]] static Runtime& instance() noexcept;

    [[nodiscard]] const SiteRef& register_site(const SiteDesc& desc);

    [[nodiscard]] Token begin(const SiteRef& site) noexcept;
    [[nodiscard]] Token start(const SiteRef& site) noexcept { return begin(site); }
    void end(Token token) noexcept;
    void stop(Token token) noexcept { end(token); }

    [[nodiscard]] bool start_async_export(const AsyncExportConfig& config = AsyncExportConfig()) noexcept;
    [[nodiscard]] ExportStats stop_async_export() noexcept;
    [[nodiscard]] ExportStats finalize(const ExportPaths& paths = ExportPaths()) noexcept;
    void reset_for_tests(const Config& config = Config()) noexcept;
    [[nodiscard]] uint64_t now_ns() const noexcept;

 private:
    Runtime();
    ~Runtime();
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
};

class Scope final {
 public:
    explicit Scope(const SiteRef& site) noexcept;
    ~Scope() noexcept;

    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

    Scope(Scope&& other) noexcept;
    Scope& operator=(Scope&&) = delete;

 private:
    Token token_;
};

namespace detail {

struct StaticSiteSlot;

[[nodiscard]] const SiteRef& static_site(StaticSiteSlot& slot, const SiteDesc& desc);

}  // namespace detail

}  // namespace perf_duration_trace

#if defined(PERF_ENABLED)

namespace perf_duration_trace::detail {

static_assert(std::is_trivially_copyable_v<Token>, "Token must remain trivially copyable");
static_assert(std::is_trivially_copyable_v<SampleRecord>,
              "SampleRecord must remain trivially copyable");

enum class AsyncState : uint8_t {
    stopped,
    running,
    stopping,
};

constexpr uint32_t kFileFormatVersion = 1U;
constexpr char kFileMagic[8] = {'P', 'D', 'T', 'B', 'I', 'N', '1', '\0'};
constexpr uint32_t kDefaultFlushIntervalMs = 100U;
constexpr size_t kDefaultShardCount = 4U;
constexpr size_t kMinimumRingCapacity = 64U;
constexpr uint64_t kOneSecondInNs = 1000000000ULL;

struct FileHeader {
    char magic[8];
    uint32_t version;
    uint32_t record_size;
    uint64_t record_count;
    uint64_t overwritten_samples;
    uint64_t dropped_samples;
    uint32_t shard_count;
    uint32_t capacity_per_shard;
};

static_assert(std::is_trivially_copyable_v<FileHeader>,
              "FileHeader must remain trivially copyable for binary serialization");
static_assert(sizeof(FileHeader) == 48U,
              "FileHeader size must match Python struct.Struct('<8sIIQQQII') = 48 bytes");
static_assert(sizeof(SampleRecord) == 32U,
              "SampleRecord size must match Python struct.Struct('<QQIIIHH') = 32 bytes");

struct ThreadRoute {
    uint64_t generation = 0;
    uint32_t tid_hash = 0;
    uint16_t shard_id = 0;
    bool initialized = false;
};

struct StaticSiteSlot {
    std::mutex mutex;
    std::atomic<const SiteRef*> site_ref {nullptr};
};

inline thread_local ThreadRoute g_thread_route;

[[nodiscard]] inline uint64_t fallback_now_ns() noexcept {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch())
                                          .count());
}

[[nodiscard]] inline uint64_t monotonic_now_ns() noexcept {
    struct timespec ts {};
#if defined(CLOCK_MONOTONIC_RAW)
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0) {
        return static_cast<uint64_t>(ts.tv_sec) * kOneSecondInNs +
               static_cast<uint64_t>(ts.tv_nsec);
    }
#endif
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
        return static_cast<uint64_t>(ts.tv_sec) * kOneSecondInNs +
               static_cast<uint64_t>(ts.tv_nsec);
    }
    return fallback_now_ns();
}

[[nodiscard]] inline uint32_t hash_current_thread() noexcept {
    const auto hashed = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    return static_cast<uint32_t>(hashed ^ (hashed >> 32U));
}

[[nodiscard]] inline size_t normalize_power_of_two(size_t value, size_t minimum) noexcept {
    size_t normalized = minimum;
    while (normalized < value) {
        normalized <<= 1U;
    }
    return normalized;
}

[[nodiscard]] inline size_t normalize_shard_count(size_t shard_count) noexcept {
    if (shard_count == 0U) {
        shard_count = static_cast<size_t>(std::thread::hardware_concurrency());
        if (shard_count == 0U) {
            shard_count = kDefaultShardCount;
        }
    }
    return normalize_power_of_two(shard_count, 1U);
}

[[nodiscard]] inline size_t normalize_capacity(size_t capacity) noexcept {
    if (capacity == 0U) {
        capacity = 4096U;
    }
    return normalize_power_of_two(capacity, kMinimumRingCapacity);
}

[[nodiscard]] inline bool sample_less(const SampleRecord& lhs, const SampleRecord& rhs) noexcept {
    if (lhs.start_ns != rhs.start_ns) {
        return lhs.start_ns < rhs.start_ns;
    }
    if (lhs.site_id != rhs.site_id) {
        return lhs.site_id < rhs.site_id;
    }
    return lhs.seq_no < rhs.seq_no;
}

struct Slot {
    std::atomic<uint64_t> sequence {0};
    SampleRecord record {};
};

class Shard final {
 public:
    explicit Shard(size_t requested_capacity)
        : capacity_(normalize_capacity(requested_capacity)),
          mask_(capacity_ - 1U),
          slots_(new Slot[capacity_]) {
        for (size_t i = 0; i < capacity_; ++i) {
            slots_[i].sequence.store(static_cast<uint64_t>(i), std::memory_order_relaxed);
        }
    }

    [[nodiscard]] bool enqueue(const SampleRecord& record) noexcept {
        auto position = enqueue_position_.load(std::memory_order_relaxed);
        for (;;) {
            Slot& slot = slots_[position & mask_];
            const auto sequence = slot.sequence.load(std::memory_order_acquire);
            const auto diff = static_cast<int64_t>(sequence) - static_cast<int64_t>(position);
            if (diff == 0) {
                if (enqueue_position_.compare_exchange_weak(
                        position, position + 1U, std::memory_order_relaxed)) {
                    slot.record = record;
                    slot.sequence.store(position + 1U, std::memory_order_release);
                    return true;
                }
                continue;
            }
            if (diff < 0) {
                dropped_samples_.fetch_add(1U, std::memory_order_relaxed);
                return false;
            }
            position = enqueue_position_.load(std::memory_order_relaxed);
        }
    }

    [[nodiscard]] bool dequeue(SampleRecord& record) noexcept {
        auto position = dequeue_position_.load(std::memory_order_relaxed);
        for (;;) {
            Slot& slot = slots_[position & mask_];
            const auto sequence = slot.sequence.load(std::memory_order_acquire);
            const auto diff = static_cast<int64_t>(sequence) - static_cast<int64_t>(position + 1U);
            if (diff == 0) {
                if (dequeue_position_.compare_exchange_weak(
                        position, position + 1U, std::memory_order_relaxed)) {
                    record = slot.record;
                    slot.sequence.store(position + static_cast<uint64_t>(capacity_), std::memory_order_release);
                    return true;
                }
                continue;
            }
            if (diff < 0) {
                return false;
            }
            position = dequeue_position_.load(std::memory_order_relaxed);
        }
    }

    [[nodiscard]] uint64_t take_dropped_samples() noexcept {
        return dropped_samples_.exchange(0U, std::memory_order_relaxed);
    }

    [[nodiscard]] size_t capacity() const noexcept { return capacity_; }

 private:
    const size_t capacity_;
    const size_t mask_;
    std::unique_ptr<Slot[]> slots_;
    std::atomic<uint64_t> enqueue_position_ {0};
    std::atomic<uint64_t> dequeue_position_ {0};
    std::atomic<uint64_t> dropped_samples_ {0};
};

class RuntimeState final {
 public:
    RuntimeState() { reconfigure(Config{}); }

    RuntimeState(const RuntimeState&) = delete;
    RuntimeState& operator=(const RuntimeState&) = delete;

    [[nodiscard]] const SiteRef& register_site(const SiteDesc& desc) {
        std::lock_guard<std::mutex> lock(site_mutex_);
        sites_.push_back(SiteRef{next_site_id_++, desc});
        return sites_.back();
    }

    [[nodiscard]] Token begin(const SiteRef& site) noexcept {
        return Token{monotonic_now_ns(), site.id};
    }

    void end(Token token) noexcept {
        if (!token.valid()) {
            return;
        }

        const auto end_ns = monotonic_now_ns();
        const auto duration_ns = end_ns >= token.start_ns ? (end_ns - token.start_ns) : 0U;
        ThreadRoute& route = route_for_current_thread();
        const auto shard_index = static_cast<size_t>(route.shard_id);
        SampleRecord record {
            token.start_ns,
            duration_ns,
            token.site_id,
            route.tid_hash,
            next_sequence_.fetch_add(1U, std::memory_order_relaxed),
            0U,
            0U,
        };
        (void)shards_[shard_index]->enqueue(record);
    }

    [[nodiscard]] bool start_async_export(const AsyncExportConfig& config) noexcept {
        std::lock_guard<std::mutex> lock(async_mutex_);
        if (async_state_ != AsyncState::stopped) {
            return false;
        }

        async_last_stats_ = make_base_stats();
        async_last_stats_ready_ = false;

        AsyncExportConfig normalized = config;
        if (normalized.flush_interval_ms == 0U) {
            normalized.flush_interval_ms = kDefaultFlushIntervalMs;
        }

        async_sample_file_ = std::fopen(normalized.paths.sample_path, "wb+");
        if (async_sample_file_ == nullptr) {
            return false;
        }

        const FileHeader header = make_header(0U, 0U);
        if (std::fwrite(&header, sizeof(header), 1U, async_sample_file_) != 1U ||
            std::fflush(async_sample_file_) != 0) {
            std::fclose(async_sample_file_);
            async_sample_file_ = nullptr;
            return false;
        }

        async_config_ = normalized;
        async_exported_samples_ = 0U;
        async_dropped_samples_ = 0U;
        async_write_failed_ = false;
        async_stop_requested_ = false;
        async_last_stats_ready_ = false;
        async_state_ = AsyncState::running;

        try {
            async_thread_ = std::thread([this]() { async_worker(); });
        } catch (...) {
            std::fclose(async_sample_file_);
            async_sample_file_ = nullptr;
            async_state_ = AsyncState::stopped;
            return false;
        }
        return true;
    }

    [[nodiscard]] ExportStats stop_async_export() noexcept {
        std::thread worker;
        {
            std::unique_lock<std::mutex> lock(async_mutex_);
            const StopAsyncAction action = prepare_stop_async_locked(worker);
            if (action == StopAsyncAction::return_last_stats) {
                return async_last_stats_;
            }
            if (action == StopAsyncAction::wait_for_completion) {
                async_cv_.wait(lock, [this]() { return async_state_ == AsyncState::stopped; });
                return async_last_stats_;
            }
        }

        return finish_stop_async(std::move(worker));
    }

    [[nodiscard]] ExportStats finalize(const ExportPaths& paths) noexcept {
        std::thread worker;
        {
            std::unique_lock<std::mutex> lock(async_mutex_);
            const StopAsyncAction action = prepare_stop_async_locked(worker);
            if (action == StopAsyncAction::return_last_stats && async_last_stats_ready_) {
                return async_last_stats_;
            }
            if (action == StopAsyncAction::wait_for_completion) {
                async_cv_.wait(lock, [this]() { return async_state_ == AsyncState::stopped; });
                return async_last_stats_;
            }
        }

        if (worker.joinable()) {
            return finish_stop_async(std::move(worker));
        }

        ExportStats stats = make_base_stats();
        std::vector<SampleRecord> merged;
        uint64_t dropped = 0U;
        drain_shards(merged, dropped);
        std::sort(merged.begin(), merged.end(), sample_less);

        FILE* sample_file = std::fopen(paths.sample_path, "wb");
        if (sample_file == nullptr) {
            return stats;
        }

        const FileHeader header = make_header(static_cast<uint64_t>(merged.size()), dropped);
        const bool header_ok = std::fwrite(&header, sizeof(header), 1U, sample_file) == 1U;
        const bool records_ok = merged.empty() ||
                                (std::fwrite(merged.data(), sizeof(SampleRecord), merged.size(), sample_file) ==
                                 merged.size());
        const bool flush_ok = std::fflush(sample_file) == 0;
        std::fclose(sample_file);
        if (!(header_ok && records_ok && flush_ok)) {
            return stats;
        }

        if (!write_site_file(paths.site_path)) {
            return stats;
        }

        stats.exported_samples = static_cast<uint64_t>(merged.size());
        stats.dropped_samples = dropped;
        stats.success = true;
        return stats;
    }

    void reset_for_tests(const Config& config) noexcept {
        (void)stop_async_export();
        reconfigure(config);
        async_last_stats_ = make_base_stats();
        async_last_stats_ready_ = false;
    }

    [[nodiscard]] uint64_t now_ns() const noexcept { return monotonic_now_ns(); }

 private:
    enum class StopAsyncAction : uint8_t {
        return_last_stats,
        wait_for_completion,
        join_worker,
    };

    [[nodiscard]] StopAsyncAction prepare_stop_async_locked(std::thread& worker) noexcept {
        if (async_state_ == AsyncState::stopped) {
            return StopAsyncAction::return_last_stats;
        }
        if (async_state_ == AsyncState::stopping) {
            return StopAsyncAction::wait_for_completion;
        }

        async_state_ = AsyncState::stopping;
        async_stop_requested_ = true;
        worker = std::move(async_thread_);
        return StopAsyncAction::join_worker;
    }

    [[nodiscard]] ExportStats finish_stop_async(std::thread worker) noexcept {

        if (worker.joinable()) {
            worker.join();
        }

        ExportStats stats = make_base_stats();
        bool ok = false;
        {
            std::lock_guard<std::mutex> lock(async_mutex_);
            stats.exported_samples = async_exported_samples_;
            stats.dropped_samples = async_dropped_samples_;
            ok = !async_write_failed_;
            ok = rewrite_async_header_locked(stats.exported_samples, stats.dropped_samples) && ok;
            ok = write_site_file(async_config_.paths.site_path) && ok;
            close_async_file_locked();
            async_stop_requested_ = false;
            async_write_failed_ = false;
            async_state_ = AsyncState::stopped;
            stats.success = ok;
            async_last_stats_ = stats;
            async_last_stats_ready_ = true;
            async_cv_.notify_all();
        }
        return stats;
    }
    void async_worker() noexcept {
        for (;;) {
            uint32_t flush_interval_ms = kDefaultFlushIntervalMs;
            {
                std::lock_guard<std::mutex> lock(async_mutex_);
                flush_interval_ms = async_config_.flush_interval_ms;
            }

            std::vector<SampleRecord> batch;
            uint64_t dropped = 0U;
            drain_shards(batch, dropped);
            if (!batch.empty() || dropped != 0U) {
                std::sort(batch.begin(), batch.end(), sample_less);
                append_async_batch(batch, dropped);
            }

            {
                std::lock_guard<std::mutex> lock(async_mutex_);
                if (async_stop_requested_) {
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(flush_interval_ms));
        }
    }

    void append_async_batch(const std::vector<SampleRecord>& batch, uint64_t dropped) noexcept {
        std::lock_guard<std::mutex> lock(async_mutex_);
        if (async_sample_file_ == nullptr) {
            return;
        }

        bool ok = true;
        if (!batch.empty()) {
            ok = std::fwrite(batch.data(), sizeof(SampleRecord), batch.size(), async_sample_file_) ==
                 batch.size();
        }
        if (ok) {
            ok = std::fflush(async_sample_file_) == 0;
        }
        if (ok) {
            async_exported_samples_ += static_cast<uint64_t>(batch.size());
        } else {
            async_write_failed_ = true;
        }
        async_dropped_samples_ += dropped;
    }

    void drain_shards(std::vector<SampleRecord>& output, uint64_t& dropped) noexcept {
        for (const auto& shard : shards_) {
            SampleRecord record {};
            while (shard->dequeue(record)) {
                output.push_back(record);
            }
            dropped += shard->take_dropped_samples();
        }
    }

    [[nodiscard]] FileHeader make_header(uint64_t exported_samples, uint64_t dropped_samples) const noexcept {
        FileHeader header {};
        std::memcpy(header.magic, kFileMagic, sizeof(kFileMagic));
        header.version = kFileFormatVersion;
        header.record_size = static_cast<uint32_t>(sizeof(SampleRecord));
        header.record_count = exported_samples;
        header.overwritten_samples = 0U;
        header.dropped_samples = dropped_samples;
        header.shard_count = static_cast<uint32_t>(shards_.size());
        header.capacity_per_shard = shards_.empty() ? 0U : static_cast<uint32_t>(shards_.front()->capacity());
        return header;
    }

    [[nodiscard]] ExportStats make_base_stats() const noexcept {
        ExportStats stats;
        stats.shard_count = static_cast<uint32_t>(shards_.size());
        stats.capacity_per_shard = shards_.empty() ? 0U : static_cast<uint32_t>(shards_.front()->capacity());
        stats.registered_sites = site_count();
        return stats;
    }

    [[nodiscard]] bool rewrite_async_header_locked(uint64_t exported_samples,
                                                   uint64_t dropped_samples) noexcept {
        if (async_sample_file_ == nullptr) {
            return false;
        }
        const FileHeader header = make_header(exported_samples, dropped_samples);
        if (std::fseek(async_sample_file_, 0, SEEK_SET) != 0) {
            return false;
        }
        const bool ok = std::fwrite(&header, sizeof(header), 1U, async_sample_file_) == 1U;
        return ok && (std::fflush(async_sample_file_) == 0);
    }

    [[nodiscard]] bool write_site_file(const char* path) const noexcept {
        FILE* site_file = std::fopen(path, "w");
        if (site_file == nullptr) {
            return false;
        }
        std::fputs("site_id\tlabel\tfile\tfunction\tline\n", site_file);
        {
            std::lock_guard<std::mutex> lock(site_mutex_);
            for (const SiteRef& site : sites_) {
                std::fprintf(site_file,
                             "%u\t%s\t%s\t%s\t%u\n",
                             site.id,
                             site.site_desc.label ? site.site_desc.label : "",
                             site.site_desc.file ? site.site_desc.file : "",
                             site.site_desc.function ? site.site_desc.function : "",
                             site.site_desc.line);
            }
        }
        return std::fclose(site_file) == 0;
    }

    void close_async_file_locked() noexcept {
        if (async_sample_file_ != nullptr) {
            std::fclose(async_sample_file_);
            async_sample_file_ = nullptr;
        }
    }

    void reconfigure(const Config& config) noexcept {
        const size_t shard_count = normalize_shard_count(config.shard_count);
        const size_t capacity = normalize_capacity(config.capacity_per_shard);

        std::vector<std::unique_ptr<Shard>> next_shards;
        next_shards.reserve(shard_count);
        for (size_t i = 0; i < shard_count; ++i) {
            next_shards.emplace_back(new Shard(capacity));
        }

        shards_.swap(next_shards);
        generation_.fetch_add(1U, std::memory_order_release);
        next_sequence_.store(0U, std::memory_order_relaxed);
    }

    [[nodiscard]] ThreadRoute& route_for_current_thread() noexcept {
        ThreadRoute& route = g_thread_route;
        const auto generation = generation_.load(std::memory_order_acquire);
        if (route.initialized && route.generation == generation &&
            static_cast<size_t>(route.shard_id) < shards_.size()) {
            return route;
        }

        route.generation = generation;
        route.tid_hash = hash_current_thread();
        route.shard_id = static_cast<uint16_t>(route.tid_hash & static_cast<uint32_t>(shards_.size() - 1U));
        route.initialized = true;
        return route;
    }

    [[nodiscard]] uint64_t site_count() const noexcept {
        std::lock_guard<std::mutex> lock(site_mutex_);
        return static_cast<uint64_t>(sites_.size());
    }

    mutable std::mutex site_mutex_;
    std::deque<SiteRef> sites_;
    std::vector<std::unique_ptr<Shard>> shards_;
    std::atomic<uint64_t> generation_ {1U};
    std::atomic<uint32_t> next_sequence_ {0U};
    uint32_t next_site_id_ = 1U;

    mutable std::mutex async_mutex_;
    std::condition_variable async_cv_;
    std::thread async_thread_;
    AsyncExportConfig async_config_ {};
    FILE* async_sample_file_ = nullptr;
    uint64_t async_exported_samples_ = 0U;
    uint64_t async_dropped_samples_ = 0U;
    ExportStats async_last_stats_ {};
    bool async_last_stats_ready_ = false;
    AsyncState async_state_ = AsyncState::stopped;
    bool async_stop_requested_ = false;
    bool async_write_failed_ = false;
};

[[nodiscard]] inline RuntimeState& state() noexcept {
    // Keep the registry alive for the whole process lifetime. PERF_TRACE_SITE stores
    // references to registered SiteRef objects, so reclaiming them during shutdown would
    // reintroduce destruction-order hazards for header-only use.
    static RuntimeState* runtime_state = new RuntimeState;
    return *runtime_state;
}

}  // namespace perf_duration_trace::detail

namespace perf_duration_trace {

[[nodiscard]] inline Runtime& Runtime::instance() noexcept {
    // Match detail::state(): the public facade is intentionally process-lifetime.
    static Runtime* runtime = new Runtime;
    return *runtime;
}

inline Runtime::Runtime() = default;
inline Runtime::~Runtime() = default;

[[nodiscard]] inline const SiteRef& Runtime::register_site(const SiteDesc& desc) {
    return detail::state().register_site(desc);
}

[[nodiscard]] inline Token Runtime::begin(const SiteRef& site) noexcept {
    return detail::state().begin(site);
}

inline void Runtime::end(Token token) noexcept {
    detail::state().end(token);
}

[[nodiscard]] inline bool Runtime::start_async_export(const AsyncExportConfig& config) noexcept {
    return detail::state().start_async_export(config);
}

[[nodiscard]] inline ExportStats Runtime::stop_async_export() noexcept {
    return detail::state().stop_async_export();
}

[[nodiscard]] inline ExportStats Runtime::finalize(const ExportPaths& paths) noexcept {
    return detail::state().finalize(paths);
}

inline void Runtime::reset_for_tests(const Config& config) noexcept {
    detail::state().reset_for_tests(config);
}

[[nodiscard]] inline uint64_t Runtime::now_ns() const noexcept {
    return detail::state().now_ns();
}

inline Scope::Scope(const SiteRef& site) noexcept : token_(Runtime::instance().begin(site)) {}

inline Scope::~Scope() noexcept {
    Runtime::instance().end(token_);
}

inline Scope::Scope(Scope&& other) noexcept : token_(other.token_) {
    other.token_ = Token{};
}

namespace detail {

[[nodiscard]] inline const SiteRef& static_site(StaticSiteSlot& slot, const SiteDesc& desc) {
    const SiteRef* site_ref = slot.site_ref.load(std::memory_order_acquire);
    if (site_ref != nullptr) {
        return *site_ref;
    }

    std::lock_guard<std::mutex> lock(slot.mutex);
    site_ref = slot.site_ref.load(std::memory_order_relaxed);
    if (site_ref == nullptr) {
        site_ref = &Runtime::instance().register_site(desc);
        slot.site_ref.store(site_ref, std::memory_order_release);
    }
    return *site_ref;
}

}  // namespace detail

}  // namespace perf_duration_trace

#else

namespace perf_duration_trace {

[[nodiscard]] inline Runtime& Runtime::instance() noexcept {
    static Runtime* runtime = new Runtime;
    return *runtime;
}

inline Runtime::Runtime() = default;
inline Runtime::~Runtime() = default;

[[nodiscard]] inline const SiteRef& Runtime::register_site(const SiteDesc& desc) {
    static const SiteRef site_ref{0U, desc};
    return site_ref;
}

[[nodiscard]] inline Token Runtime::begin(const SiteRef&) noexcept { return {}; }
inline void Runtime::end(Token) noexcept {}
[[nodiscard]] inline bool Runtime::start_async_export(const AsyncExportConfig&) noexcept { return true; }
[[nodiscard]] inline ExportStats Runtime::stop_async_export() noexcept {
    ExportStats stats;
    stats.success = true;
    return stats;
}
[[nodiscard]] inline ExportStats Runtime::finalize(const ExportPaths&) noexcept {
    ExportStats stats;
    stats.success = true;
    return stats;
}
inline void Runtime::reset_for_tests(const Config&) noexcept {}
[[nodiscard]] inline uint64_t Runtime::now_ns() const noexcept { return 0U; }

inline Scope::Scope(const SiteRef&) noexcept : token_{} {}
inline Scope::~Scope() noexcept = default;
inline Scope::Scope(Scope&& other) noexcept : token_(other.token_) { other.token_ = Token{}; }

namespace detail {

struct StaticSiteSlot {};

[[nodiscard]] inline const SiteRef& static_site(StaticSiteSlot&, const SiteDesc& desc) {
    static const SiteRef site_ref{0U, desc};
    return site_ref;
}

}  // namespace detail

}  // namespace perf_duration_trace

#endif

#if defined(PERF_ENABLED)

#if defined(__GNUC__) || defined(__clang__)
#define PERF_DURATION_TRACE_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#define PERF_DURATION_TRACE_FUNCTION __FUNCSIG__
#else
#define PERF_DURATION_TRACE_FUNCTION __func__
#endif

#define PERF_DURATION_TRACE_JOIN_INNER(a, b) a##b
#define PERF_DURATION_TRACE_JOIN(a, b) PERF_DURATION_TRACE_JOIN_INNER(a, b)

#define PERF_TRACE_SITE(name_literal)                                                        \
    ([]() -> const ::perf_duration_trace::SiteRef& {                                         \
        static ::perf_duration_trace::detail::StaticSiteSlot perf_static_site_slot;          \
        return ::perf_duration_trace::detail::static_site(                                   \
            perf_static_site_slot,                                                           \
            ::perf_duration_trace::SiteDesc{                                                 \
                name_literal, __FILE__, PERF_DURATION_TRACE_FUNCTION,                        \
                static_cast<uint32_t>(__LINE__)});                                           \
    }())

#define PERF_TRACE_SCOPE(name_literal)                                                       \
    ::perf_duration_trace::Scope PERF_DURATION_TRACE_JOIN(perf_scope_, __LINE__)(            \
        PERF_TRACE_SITE(name_literal))

#define PERF_TRACE_SCOPE_FUNC() PERF_TRACE_SCOPE(PERF_DURATION_TRACE_FUNCTION)

#define PERF_TRACE_BEGIN(name_literal)                                                       \
    (::perf_duration_trace::Runtime::instance().begin(PERF_TRACE_SITE(name_literal)))

#define PERF_TRACE_END(token) (::perf_duration_trace::Runtime::instance().end((token)))

#define PERF_SCOPE(name_literal) PERF_TRACE_SCOPE(name_literal)
#define PERF_SCOPE_FUNC() PERF_TRACE_SCOPE_FUNC()
#define PERF_BEGIN(name_literal) PERF_TRACE_BEGIN(name_literal)
#define PERF_END(token) PERF_TRACE_END(token)

#else

#define PERF_TRACE_SCOPE(name_literal) ((void)sizeof(name_literal))
#define PERF_TRACE_SCOPE_FUNC() ((void)0)
#define PERF_TRACE_BEGIN(name_literal) (::perf_duration_trace::Token{})
#define PERF_TRACE_END(token) ((void)(token))

#define PERF_SCOPE(name_literal) PERF_TRACE_SCOPE(name_literal)
#define PERF_SCOPE_FUNC() PERF_TRACE_SCOPE_FUNC()
#define PERF_BEGIN(name_literal) PERF_TRACE_BEGIN(name_literal)
#define PERF_END(token) PERF_TRACE_END(token)

#endif
