#include "perf_duration_trace.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

void perf_duration_trace_multi_tu_site_a();
void perf_duration_trace_multi_tu_site_b();

namespace {

using perf_duration_trace::AsyncExportConfig;
using perf_duration_trace::ExportStats;
using perf_duration_trace::Runtime;
using perf_duration_trace::SampleRecord;

struct BinaryHeader {
    char magic[8];
    uint32_t version;
    uint32_t record_size;
    uint64_t record_count;
    uint64_t overwritten_samples;
    uint64_t dropped_samples;
    uint32_t shard_count;
    uint32_t capacity_per_shard;
};

[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error(message);
}

void expect(bool cond, const std::string& message) {
    if (!cond) {
        fail(message);
    }
}

BinaryHeader read_header(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    expect(in.good(), "failed to open binary export: " + path);
    BinaryHeader header {};
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    expect(in.good(), "failed to read binary header");
    return header;
}

std::vector<SampleRecord> read_records(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    expect(in.good(), "failed to open binary export: " + path);
    in.seekg(static_cast<std::streamoff>(sizeof(BinaryHeader)), std::ios::beg);

    std::vector<SampleRecord> records;
    SampleRecord record {};
    while (in.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        records.push_back(record);
    }
    return records;
}

std::vector<std::string> read_lines(const std::string& path) {
    std::ifstream in(path);
    expect(in.good(), "failed to open text export: " + path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(in, line)) {
        lines.push_back(line);
    }
    return lines;
}

size_t count_lines_with_substring(const std::vector<std::string>& lines, const std::string& needle) {
    size_t count = 0U;
    for (const std::string& line : lines) {
        if (line.find(needle) != std::string::npos) {
            ++count;
        }
    }
    return count;
}

void expect_records_sorted(const std::vector<SampleRecord>& records) {
    for (size_t i = 1U; i < records.size(); ++i) {
        const SampleRecord& prev = records[i - 1U];
        const SampleRecord& curr = records[i];
        const bool in_order =
            (prev.start_ns < curr.start_ns) ||
            (prev.start_ns == curr.start_ns && prev.site_id < curr.site_id) ||
            (prev.start_ns == curr.start_ns && prev.site_id == curr.site_id && prev.seq_no <= curr.seq_no);
        expect(in_order, "records should be sorted for analyzer consumption");
    }
}

void reset_runtime(size_t shards = 4U, size_t capacity = 128U) {
    Runtime::instance().reset_for_tests({shards, capacity});
}

void concurrent_registration_site() {
    PERF_SCOPE("concurrent_registration_case");
}

void concurrent_registration_site_0() {
    PERF_SCOPE("concurrent_registration_case_0");
}

void concurrent_registration_site_1() {
    PERF_SCOPE("concurrent_registration_case_1");
}

void concurrent_registration_site_2() {
    PERF_SCOPE("concurrent_registration_case_2");
}

void concurrent_registration_site_3() {
    PERF_SCOPE("concurrent_registration_case_3");
}

void concurrent_registration_site_4() {
    PERF_SCOPE("concurrent_registration_case_4");
}

void concurrent_registration_site_5() {
    PERF_SCOPE("concurrent_registration_case_5");
}

void concurrent_registration_site_6() {
    PERF_SCOPE("concurrent_registration_case_6");
}

void concurrent_registration_site_7() {
    PERF_SCOPE("concurrent_registration_case_7");
}

void test_scope_and_manual_modes() {
    reset_runtime();
    const std::string bin_path = "/tmp/perf_duration_trace_scope.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_scope.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    {
        PERF_SCOPE("scope_case");
    }
    auto token = PERF_BEGIN("manual_case");
    PERF_END(token);

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "finalize should succeed");
    expect(stats.exported_samples == 2U, "expected two exported samples");
    expect(stats.dropped_samples == 0U, "no samples should be dropped in basic case");

    const BinaryHeader header = read_header(bin_path);
    expect(std::memcmp(header.magic, "PDTBIN1", sizeof(header.magic)) == 0, "unexpected file magic");
    expect(header.record_count == 2U, "binary header record count mismatch");
    expect(header.dropped_samples == 0U, "binary header dropped count mismatch");

    const auto site_lines = read_lines(site_path);
    expect(site_lines.size() >= 3U, "site export should include header and two sites");
}

perf_duration_trace::Token pass_through_token() {
    return PERF_BEGIN("async_like_case");
}

void finish_token(perf_duration_trace::Token token) {
    PERF_END(token);
}

void test_cross_function_manual_token() {
    reset_runtime();
    const std::string bin_path = "/tmp/perf_duration_trace_cross.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_cross.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    auto token = pass_through_token();
    finish_token(token);

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "finalize should succeed for cross-function token");
    expect(stats.exported_samples == 1U, "expected one cross-function sample");

    const auto records = read_records(bin_path);
    expect(records.size() == 1U, "expected one binary record");
    expect(records[0].site_id != 0U, "site id should be populated");
}

void test_cross_thread_manual_token() {
    reset_runtime();
    const std::string bin_path = "/tmp/perf_duration_trace_cross_thread.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_cross_thread.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    std::promise<perf_duration_trace::Token> token_promise;
    std::future<perf_duration_trace::Token> token_future = token_promise.get_future();

    std::thread producer([&token_promise]() { token_promise.set_value(PERF_BEGIN("cross_thread_case")); });
    std::thread consumer([token_future = std::move(token_future)]() mutable { finish_token(token_future.get()); });
    producer.join();
    consumer.join();

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "finalize should succeed for cross-thread token");
    expect(stats.exported_samples == 1U, "expected one cross-thread sample");

    const auto records = read_records(bin_path);
    expect(records.size() == 1U, "expected one cross-thread binary record");
    expect(records[0].site_id != 0U, "cross-thread sample should keep its site id");
}

void test_multithread_capture() {
    reset_runtime(8U, 256U);
    const std::string bin_path = "/tmp/perf_duration_trace_mt.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_mt.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    constexpr int kThreadCount = 4;
    constexpr int kIterations = 50;
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(kThreadCount));
    for (int t = 0; t < kThreadCount; ++t) {
        threads.emplace_back([]() {
            for (int i = 0; i < kIterations; ++i) {
                PERF_SCOPE("threaded_case");
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "finalize should succeed for multithread capture");
    expect(stats.exported_samples == static_cast<uint64_t>(kThreadCount * kIterations),
           "unexpected multithread sample count");
    expect(stats.dropped_samples == 0U, "multithread case should fit in queue");
}

void test_drop_newest_when_full() {
    reset_runtime(1U, 64U);
    const std::string bin_path = "/tmp/perf_duration_trace_drop.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_drop.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    for (int i = 0; i < 200; ++i) {
        PERF_SCOPE("drop_case");
    }

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "finalize should succeed for drop case");
    expect(stats.exported_samples == 64U, "bounded queue should retain up to its capacity");
    expect(stats.dropped_samples == 136U, "drop count should match queue overflow");

    const BinaryHeader header = read_header(bin_path);
    expect(header.record_count == 64U, "binary header record count mismatch for drop case");
    expect(header.dropped_samples == 136U, "binary header dropped count mismatch");
}

void test_async_export() {
    reset_runtime(4U, 128U);
    const std::string bin_path = "/tmp/perf_duration_trace_async.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_async.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    const bool started = Runtime::instance().start_async_export({{bin_path.c_str(), site_path.c_str()}, 10U});
    expect(started, "async export should start");

    for (int i = 0; i < 120; ++i) {
        PERF_SCOPE("async_flush_case");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const ExportStats stats = Runtime::instance().stop_async_export();
    expect(stats.success, "async stop should succeed");
    expect(stats.exported_samples == 120U, "async export should flush all produced samples");
    expect(stats.dropped_samples == 0U, "async export should not drop in bounded test");

    const BinaryHeader header = read_header(bin_path);
    expect(header.record_count == 120U, "async header record count mismatch");
    expect(header.dropped_samples == 0U, "async header dropped count mismatch");

    const auto records = read_records(bin_path);
    expect(records.size() == 120U, "async binary record count mismatch");
    const auto site_lines = read_lines(site_path);
    expect(site_lines.size() >= 2U, "async site export should include header and data");
}

void test_finalize_stops_async_export() {
    reset_runtime(4U, 128U);
    const std::string bin_path = "/tmp/perf_duration_trace_async_finalize.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_async_finalize.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    const bool started = Runtime::instance().start_async_export({{bin_path.c_str(), site_path.c_str()}, 10U});
    expect(started, "async export should start for finalize test");

    for (int i = 0; i < 96; ++i) {
        PERF_SCOPE("async_finalize_case");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "finalize should stop async export and succeed");
    expect(stats.exported_samples == 96U, "finalize should flush all async samples");

    const BinaryHeader header = read_header(bin_path);
    expect(header.record_count == 96U, "finalize should rewrite the async header");

    const ExportStats repeated_stop = Runtime::instance().stop_async_export();
    expect(repeated_stop.success, "stop after finalize should return the cached async result");
    expect(repeated_stop.exported_samples == stats.exported_samples,
           "stop after finalize should report the same exported sample count");
}

void test_export_records_are_sorted() {
    reset_runtime(4U, 128U);
    const std::string bin_path = "/tmp/perf_duration_trace_sorted.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_sorted.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    for (int i = 0; i < 32; ++i) {
        PERF_SCOPE("sorted_case");
    }

    const ExportStats sorted_stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(sorted_stats.success, "sorted export finalize should succeed");

    const auto records = read_records(bin_path);
    expect(records.size() == 32U, "sorted export should keep all records");
    expect_records_sorted(records);
}

void test_finalize_after_async_stop_keeps_export_count_stable() {
    reset_runtime(4U, 128U);
    const std::string async_bin_path = "/tmp/perf_duration_trace_sorted_async.perfbin";
    const std::string async_site_path = "/tmp/perf_duration_trace_sorted_async.sites.tsv";
    std::remove(async_bin_path.c_str());
    std::remove(async_site_path.c_str());

    const bool started =
        Runtime::instance().start_async_export({{async_bin_path.c_str(), async_site_path.c_str()}, 10U});
    expect(started, "async stability export should start");

    for (int i = 0; i < 96; ++i) {
        PERF_SCOPE("sorted_async_stability_case");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    const ExportStats stopped_stats = Runtime::instance().stop_async_export();
    expect(stopped_stats.success, "async stability stop should succeed");

    const BinaryHeader stopped_header = read_header(async_bin_path);
    expect(stopped_header.record_count == stopped_stats.exported_samples,
           "async stability stop should persist exported sample count into header");

    const ExportStats finalize_after_stop =
        Runtime::instance().finalize({async_bin_path.c_str(), async_site_path.c_str()});
    expect(finalize_after_stop.success, "async stability finalize after stop should succeed");
    expect(finalize_after_stop.exported_samples == stopped_stats.exported_samples,
           "async stability finalize after stop should keep exported sample count stable");

    const BinaryHeader finalized_header = read_header(async_bin_path);
    expect(finalized_header.record_count == stopped_stats.exported_samples,
           "async stability finalize after stop should not rewrite header record count");
}

void test_failed_async_start_clears_cached_async_stats() {
    reset_runtime(4U, 128U);
    const std::string seed_bin_path = "/tmp/perf_duration_trace_async_seed.perfbin";
    const std::string seed_site_path = "/tmp/perf_duration_trace_async_seed.sites.tsv";
    std::remove(seed_bin_path.c_str());
    std::remove(seed_site_path.c_str());

    const bool started = Runtime::instance().start_async_export({{seed_bin_path.c_str(), seed_site_path.c_str()}, 10U});
    expect(started, "seed async export should start");
    PERF_SCOPE("async_seed_case");
    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    const ExportStats seed_stats = Runtime::instance().stop_async_export();
    expect(seed_stats.success, "seed async stop should succeed");

    const bool failed_start = Runtime::instance().start_async_export(
        {{"/tmp/perf_duration_trace_missing_dir/trace.perfbin",
          "/tmp/perf_duration_trace_missing_dir/trace.sites.tsv"},
         10U});
    expect(!failed_start, "async export should fail for a missing directory");

    const ExportStats after_failed_start = Runtime::instance().stop_async_export();
    expect(!after_failed_start.success,
           "failed async start should not report a cached successful async result");
    expect(after_failed_start.exported_samples == 0U,
           "failed async start should clear cached exported sample count");
    expect(after_failed_start.dropped_samples == 0U,
           "failed async start should clear cached dropped sample count");

    const std::string sync_bin_path = "/tmp/perf_duration_trace_sync_after_failed_start.perfbin";
    const std::string sync_site_path = "/tmp/perf_duration_trace_sync_after_failed_start.sites.tsv";
    std::remove(sync_bin_path.c_str());
    std::remove(sync_site_path.c_str());

    {
        PERF_SCOPE("sync_after_failed_start_case");
    }
    const ExportStats sync_stats = Runtime::instance().finalize({sync_bin_path.c_str(), sync_site_path.c_str()});
    expect(sync_stats.success, "sync finalize after failed async start should succeed");
    expect(sync_stats.exported_samples == 1U,
           "sync finalize after failed async start should export the current sample");
}

void test_concurrent_stop_async_export() {
    reset_runtime(4U, 128U);
    const std::string bin_path = "/tmp/perf_duration_trace_async_stop.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_async_stop.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    const bool started = Runtime::instance().start_async_export({{bin_path.c_str(), site_path.c_str()}, 10U});
    expect(started, "async export should start for concurrent stop test");

    for (int i = 0; i < 80; ++i) {
        PERF_SCOPE("async_concurrent_stop_case");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(30));

    ExportStats stats1 {};
    ExportStats stats2 {};
    std::thread t1([&stats1]() { stats1 = Runtime::instance().stop_async_export(); });
    std::thread t2([&stats2]() { stats2 = Runtime::instance().stop_async_export(); });
    t1.join();
    t2.join();

    expect(stats1.success, "first concurrent stop should succeed");
    expect(stats2.success, "second concurrent stop should succeed");
    expect(stats1.exported_samples == stats2.exported_samples,
           "concurrent stop callers should observe the same exported sample count");

    const BinaryHeader header = read_header(bin_path);
    expect(header.record_count == stats1.exported_samples,
           "concurrent stop should write a consistent final header");
}

void test_concurrent_static_site_registration() {
    reset_runtime(8U, 128U);
    const std::string bin_path = "/tmp/perf_duration_trace_registration.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_registration.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    constexpr int kThreadCount = 8;
    std::atomic<bool> start {false};
    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(kThreadCount));
    for (int i = 0; i < kThreadCount; ++i) {
        threads.emplace_back([&start]() {
            while (!start.load(std::memory_order_acquire)) {
            }
            concurrent_registration_site();
        });
    }
    start.store(true, std::memory_order_release);
    for (auto& thread : threads) {
        thread.join();
    }

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "concurrent registration finalize should succeed");
    expect(stats.exported_samples == static_cast<uint64_t>(kThreadCount),
           "concurrent registration should record one sample per thread");
}

void test_concurrent_distinct_static_site_registration() {
    reset_runtime(8U, 128U);
    const std::string bin_path = "/tmp/perf_duration_trace_registration_distinct.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_registration_distinct.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    using SiteFn = void (*)();
    constexpr SiteFn kSiteFns[] = {
        concurrent_registration_site_0,
        concurrent_registration_site_1,
        concurrent_registration_site_2,
        concurrent_registration_site_3,
        concurrent_registration_site_4,
        concurrent_registration_site_5,
        concurrent_registration_site_6,
        concurrent_registration_site_7,
    };
    constexpr const char* kLabels[] = {
        "concurrent_registration_case_0",
        "concurrent_registration_case_1",
        "concurrent_registration_case_2",
        "concurrent_registration_case_3",
        "concurrent_registration_case_4",
        "concurrent_registration_case_5",
        "concurrent_registration_case_6",
        "concurrent_registration_case_7",
    };

    std::atomic<bool> start {false};
    std::vector<std::thread> threads;
    threads.reserve(sizeof(kSiteFns) / sizeof(kSiteFns[0]));
    for (SiteFn site_fn : kSiteFns) {
        threads.emplace_back([&start, site_fn]() {
            while (!start.load(std::memory_order_acquire)) {
            }
            site_fn();
        });
    }
    start.store(true, std::memory_order_release);
    for (auto& thread : threads) {
        thread.join();
    }

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "distinct concurrent registration finalize should succeed");
    expect(stats.exported_samples == sizeof(kSiteFns) / sizeof(kSiteFns[0]),
           "distinct concurrent registration should record one sample per thread");

    const auto site_lines = read_lines(site_path);
    for (const char* label : kLabels) {
        expect(count_lines_with_substring(site_lines, label) == 1U,
               std::string("missing distinct site label: ") + label);
    }
}

void test_multi_translation_unit_site_registration() {
    reset_runtime();
    const std::string bin_path = "/tmp/perf_duration_trace_multi_tu.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_multi_tu.sites.tsv";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());

    perf_duration_trace_multi_tu_site_a();
    perf_duration_trace_multi_tu_site_b();

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "multi translation unit finalize should succeed");
    expect(stats.exported_samples == 2U, "expected one sample from each translation unit");

    const auto site_lines = read_lines(site_path);
    expect(count_lines_with_substring(site_lines, "multi_tu_site_a") == 1U,
           "expected the first translation unit site metadata");
    expect(count_lines_with_substring(site_lines, "multi_tu_site_b") == 1U,
           "expected the second translation unit site metadata");
}

void test_binary_header_round_trip_matches_python_parser() {
    reset_runtime(4U, 128U);
    const std::string bin_path = "/tmp/perf_duration_trace_roundtrip.perfbin";
    const std::string site_path = "/tmp/perf_duration_trace_roundtrip.sites.tsv";
    const std::string json_path = "/tmp/perf_duration_trace_roundtrip.json";
    std::remove(bin_path.c_str());
    std::remove(site_path.c_str());
    std::remove(json_path.c_str());

    for (int i = 0; i < 10; ++i) {
        PERF_SCOPE("roundtrip_case");
    }

    const ExportStats stats = Runtime::instance().finalize({bin_path.c_str(), site_path.c_str()});
    expect(stats.success, "roundtrip finalize should succeed");

    const std::string command =
        "python3 projects/perf_duration_trace/tools/perf_duration_analyze.py " + bin_path + " " + site_path +
        " --json > " + json_path;
    const int exit_code = std::system(command.c_str());
    expect(exit_code == 0, "analyzer command should succeed for roundtrip test");

    std::ifstream in(json_path);
    expect(in.good(), "analyzer should write json output for roundtrip test");
    std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    expect(content.find("\"record_count\": 10") != std::string::npos,
           "Python parser should report exactly 10 records from roundtrip binary");
    expect(content.find("\"dropped\": 0") != std::string::npos,
           "Python parser should report 0 dropped samples from roundtrip binary");
}

}  // namespace

int main() {
    try {
        test_scope_and_manual_modes();
        test_cross_function_manual_token();
        test_cross_thread_manual_token();
        test_multithread_capture();
        test_drop_newest_when_full();
        test_async_export();
        test_finalize_stops_async_export();
        test_export_records_are_sorted();
        test_finalize_after_async_stop_keeps_export_count_stable();
        test_failed_async_start_clears_cached_async_stats();
        test_concurrent_stop_async_export();
        test_concurrent_static_site_registration();
        test_concurrent_distinct_static_site_registration();
        test_multi_translation_unit_site_registration();
        test_binary_header_round_trip_matches_python_parser();
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "perf_duration_trace_test failed: %s\n", ex.what());
        return 1;
    }

    std::puts("perf_duration_trace_test passed");
    return 0;
}
