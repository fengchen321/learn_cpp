#include <benchmark/benchmark.h>

#include "perf_duration_trace.h"

static void BM_perf_trace_disabled_baseline(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_perf_trace_disabled_baseline);

static void BM_perf_trace_disabled_scope(benchmark::State& state) {
    for (auto _ : state) {
        PERF_SCOPE("bench_disabled_scope");
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_perf_trace_disabled_scope);

static void BM_perf_trace_disabled_manual(benchmark::State& state) {
    for (auto _ : state) {
        auto token = PERF_BEGIN("bench_disabled_manual");
        benchmark::ClobberMemory();
        PERF_END(token);
    }
}
BENCHMARK(BM_perf_trace_disabled_manual);

int main(int argc, char** argv) {
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) {
        return 1;
    }
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
