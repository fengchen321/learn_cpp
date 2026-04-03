#include "perf_duration_trace.h"

#include <cstdio>
#include <stdexcept>
#include <string>

namespace {

[[noreturn]] void fail(const std::string& message) {
    throw std::runtime_error(message);
}

void expect(bool cond, const std::string& message) {
    if (!cond) {
        fail(message);
    }
}

void test_disabled_mode_is_noop() {
    PERF_SCOPE("disabled_scope_case");
    auto token = PERF_BEGIN("disabled_manual_case");
    PERF_END(token);

    const bool started = perf_duration_trace::Runtime::instance().start_async_export();
    expect(started, "disabled mode start_async_export should succeed");

    const auto stop_stats = perf_duration_trace::Runtime::instance().stop_async_export();
    expect(stop_stats.success, "disabled mode stop_async_export should succeed");
    expect(stop_stats.exported_samples == 0U, "disabled stop should export zero samples");
    expect(stop_stats.dropped_samples == 0U, "disabled stop should drop zero samples");

    const auto finalize_stats = perf_duration_trace::Runtime::instance().finalize();
    expect(finalize_stats.success, "disabled mode finalize should succeed");
    expect(finalize_stats.exported_samples == 0U, "disabled finalize should export zero samples");
    expect(finalize_stats.dropped_samples == 0U, "disabled finalize should drop zero samples");
}

}  // namespace

int main() {
    try {
        test_disabled_mode_is_noop();
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "perf_duration_trace_disabled_test failed: %s\n", ex.what());
        return 1;
    }

    std::puts("perf_duration_trace_disabled_test passed");
    return 0;
}
