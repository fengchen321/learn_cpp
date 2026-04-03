//
// Created by lsfco on 2024/9/7.
//
#include <iostream>
#include <vector>
#include "print.h"
#include "ScopeProfiler.h"
#include <benchmark/benchmark.h>

void test_ScopeProfiler() {
    int sum = 0;
    std::vector<int> arr;
    arr.resize(1'000'000); //1 million elements
    for (size_t i = 0; i < arr.size(); i++) {
        arr[i] = i % 10;
    }
    {
        sum = 0;
        ScopeProfiler _("下标遍历");
        for (size_t i = 0; i < arr.size(); i++) {
            sum += arr[i] % 10;
        }
    }
    {
        sum = 0;
        ScopeProfiler _("range遍历");
        for (auto const &a : arr) {
            sum += a % 10;
        }
    }
    {
        sum = 0;
        ScopeProfiler _("迭代器遍历");
        for (auto it = arr.begin(); it!= arr.end(); ++it) {
            sum += *it %10;
        }
    }
    {
        sum = 0;
        ScopeProfiler _("ָ指针遍历");
        int *p = arr.data(), *end = p + arr.size();
        while (p != end) {
            sum += *p % 10;
            p++;
        }
    }
    printScopeProfiler();
}


static void BM_subscript(benchmark::State& state) {
    std::vector<int> arr(state.range(0));
    for (size_t i = 0; i < arr.size(); i++) {
        arr[i] = i % 10;
    }
    
    for (auto _ : state) {
        int sum = 0;
        for (size_t i = 0; i < arr.size(); i++) {
            sum += arr[i] % 10;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_subscript)->Arg(1'000'000);

static void BM_range(benchmark::State& state) {
    std::vector<int> arr(state.range(0));
    for (size_t i = 0; i < arr.size(); i++) {
        arr[i] = i % 10;
    }
    
    for (auto _ : state) {
        int sum = 0;
        for (auto const &a : arr) {
            sum += a % 10;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_range)->Arg(1'000'000);

static void BM_iterator(benchmark::State& state) {
    std::vector<int> arr(state.range(0));
    for (size_t i = 0; i < arr.size(); i++) {
        arr[i] = i % 10;
    }
    
    for (auto _ : state) {
        int sum = 0;
        for (auto it = arr.begin(); it != arr.end(); ++it) {
            sum += *it % 10;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_iterator)->Arg(1'000'000);

static void BM_pointer(benchmark::State& state) {
    std::vector<int> arr(state.range(0));
    for (size_t i = 0; i < arr.size(); i++) {
        arr[i] = i % 10;
    }
    
    for (auto _ : state) {
        int sum = 0;
        int *p = arr.data();
        int *end = p + arr.size();
        while (p != end) {
            sum += *p % 10;
            p++;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_pointer)->Arg(1'000'000);


int main(int argc, char** argv) {
    test_ScopeProfiler();
    
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    return 0;
}
// BENCHMARK_MAIN();