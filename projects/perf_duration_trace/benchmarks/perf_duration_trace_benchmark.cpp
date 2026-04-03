#include <benchmark/benchmark.h>

#define PERF_ENABLED 1
#include "perf_duration_trace.h"

namespace {

void reset_runtime_for_benchmark() {
    perf_duration_trace::Runtime::instance().reset_for_tests({4U, 1U << 20U});
}

}  // namespace

static void BM_perf_trace_baseline(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_perf_trace_baseline);

static void BM_perf_trace_scope(benchmark::State& state) {
    reset_runtime_for_benchmark();
    for (auto _ : state) {
        PERF_SCOPE("bench_scope");
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_perf_trace_scope);

static void BM_perf_trace_manual(benchmark::State& state) {
    reset_runtime_for_benchmark();
    for (auto _ : state) {
        auto token = PERF_BEGIN("bench_manual");
        benchmark::ClobberMemory();
        PERF_END(token);
    }
}
BENCHMARK(BM_perf_trace_manual);

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
