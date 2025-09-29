#include "kernel_map_generate.h"
#include "kernel_map.h"
#include <iostream>
#include <benchmark/benchmark.h>
// setarch `uname -m` -R ./test_kernel_map 1000000 100
int COUNT;
size_t SELECT_COUNT;
std::vector<std::string> kernel_names;
std::vector<std::string> select_name;
std::vector<size_t> fix_index;

template <typename MapType>
void BM_LargeScale(benchmark::State& state) {
    MapType kmap;
    for (auto _ : state) {
        for (size_t i = 0; i < COUNT; ++i) {
            benchmark::DoNotOptimize(kmap.AddFuncPtrAndNameStr(kernel_names[i].c_str()));
        }
    }
    state.SetItemsProcessed(COUNT);
}

template <typename MapType>
void BM_SelectScale(benchmark::State& state) {
    MapType kmap;
    for (auto _ : state) {
        for (size_t i = 0; i < COUNT; ++i) {
            size_t idx = fix_index[i];
            benchmark::DoNotOptimize(kmap.AddFuncPtrAndNameStr(select_name[idx].c_str()));
        }
    }
    state.SetItemsProcessed(COUNT);
}

// 注册测试
BENCHMARK_TEMPLATE(BM_LargeScale, KernelNameMap_PtrVersion)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_LargeScale, KernelNameMap_StrVersion)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_LargeScale, KernelNameMap_memcpyVersion)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_LargeScale, KernelNameUnorderMap_StrVersion)->Unit(benchmark::kMillisecond);


BENCHMARK_TEMPLATE(BM_SelectScale, KernelNameMap_PtrVersion)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SelectScale, KernelNameMap_StrVersion)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SelectScale, KernelNameMap_memcpyVersion)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_SelectScale, KernelNameUnorderMap_StrVersion)->Unit(benchmark::kMillisecond);


int main(int argc, char** argv) {
    COUNT = 100000;
    SELECT_COUNT = 100;
    if (argc > 1) COUNT = std::stoi(argv[1]);
    if (argc > 2) SELECT_COUNT = std::stoi(argv[2]);
    if (COUNT <= 0 || SELECT_COUNT <= 0 || SELECT_COUNT > COUNT) {
        std::cerr << "Usage: " << argv[0] << " [COUNT=100000] [SELECT_COUNT=100]\n";
        return 1;
    }

    // 初始化测试数据
    kernel_names = GenerateUniqueKernelNames(COUNT);
    select_name = selectRandomElements(kernel_names, SELECT_COUNT);
    fix_index = fixed_indices(COUNT, SELECT_COUNT);

    // 运行基准测试
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    return 0;
}
