// https://www.cnblogs.com/ho966/p/17084350.html
#include <benchmark/benchmark.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#if __cplusplus >= 202302L
#include <stacktrace>
#endif

/*  gcc内置函数 */
/***********************************************/
#define GCCBT(i) \
    if(i >= size) return i;\
    frameNow = __builtin_frame_address(i);\
    if((unsigned long)frameNow <= (unsigned long)frameLast ||\
    ((unsigned long)frameLast != 0 && (unsigned long)frameNow > (unsigned long)frameLast + (1ULL<<24)))\
        return i;\
    frameLast = frameNow;\
    stack[i] = __builtin_extract_return_addr(__builtin_return_address(i));\

static int gccBacktrace(void** stack, int size)
{
    void* frameNow = 0; 
    void* frameLast = 0;
    GCCBT(0); GCCBT(1); GCCBT(2); GCCBT(3); GCCBT(4); GCCBT(5); GCCBT(6); GCCBT(7);
    GCCBT(8); GCCBT(9); GCCBT(10); GCCBT(11); GCCBT(12); GCCBT(13); GCCBT(14); GCCBT(15);
    GCCBT(16); GCCBT(17); GCCBT(18); GCCBT(19); GCCBT(20); GCCBT(21); GCCBT(22); GCCBT(23);
    GCCBT(24); GCCBT(25); GCCBT(26); GCCBT(27); GCCBT(28); GCCBT(29); GCCBT(30); GCCBT(31);
    return 32;
}

/*  内置汇编获取寄存器返回值 */
/***********************************************/
static int asmBacktrace(void** stack, int size)
{
#ifndef __x86_64__
    return 0;
#else
    int frame = 0;
    void ** ebp;
    __asm__ __volatile__("mov %%rbp, %0;\n\t" : "=m"(ebp) ::"memory");
    while (ebp && frame < size
        && (unsigned long long)(*ebp) < (unsigned long long)(ebp) + (1ULL << 24)//16M
        && (unsigned long long)(*ebp) > (unsigned long long)(ebp))
    {
        stack[frame++] = *(ebp + 1);
        ebp = (void**)(*ebp);
    }
    return frame;
#endif
}

#if __cplusplus >= 202302L
static int stacktraceBacktrace(void** stack, int size)
{
    auto trace = std::stacktrace::current();
    size_t trace_size = std::min(static_cast<size_t>(size), trace.size());
    
    for (size_t i = 0; i < trace_size; ++i) {
        if (i < static_cast<size_t>(size)) {
            stack[i] = reinterpret_cast<void*>(trace[i].native_handle());
        }
    }
    
    return static_cast<int>(trace_size);
}
#endif

// 单独的验证函数，不在benchmark中使用
static void print_backtrace_comparison() {
    void* buffer[16];
    
    printf("\n=== BACKTRACE COMPARISON ===\n");
    
    // libc backtrace
    int libc_size = backtrace(buffer, 16);
    printf("libc backtrace (%d frames):\n", libc_size);
    char **strings = backtrace_symbols(buffer, libc_size);
    for (int i = 0; i < libc_size && i < 8; ++i) {
        printf("  [%d] %p %s\n", i, buffer[i], strings[i]);
    }
    free(strings);
    
    // gcc backtrace
    int gcc_size = gccBacktrace(buffer, 16);
    printf("gcc backtrace (%d frames):\n", gcc_size);
    for (int i = 0; i < gcc_size && i < 8; ++i) {
        printf("  [%d] %p\n", i, buffer[i]);
    }
    
    // asm backtrace
    int asm_size = asmBacktrace(buffer, 16);
    printf("asm backtrace (%d frames):\n", asm_size);
    for (int i = 0; i < asm_size && i < 8; ++i) {
        printf("  [%d] %p\n", i, buffer[i]);
    }
#if __cplusplus >= 202302L
    // C++23 stacktrace
    auto st = std::stacktrace::current();
    printf("C++23 stacktrace (%zu frames):\n", st.size());
    for (size_t i = 0; i < st.size() && i < 8; ++i) {
        printf("  [%zu] %s\n", i, st[i].description().c_str());
    }
#endif
    
    printf("============================\n\n");
}

void __attribute__((noinline)) func5(void** buffer, int (*backtrace_func)(void**, int))
{
    backtrace_func(buffer, 16);
}

void __attribute__((noinline)) func4(void** buffer, int (*backtrace_func)(void**, int))
{
    func5(buffer, backtrace_func);
}

void __attribute__((noinline)) func3(void** buffer, int (*backtrace_func)(void**, int))
{
    func4(buffer, backtrace_func);
}

void __attribute__((noinline)) func2(void** buffer, int (*backtrace_func)(void**, int))
{
    func3(buffer, backtrace_func);
}

void __attribute__((noinline)) func1(void** buffer, int (*backtrace_func)(void**, int))
{
    func2(buffer, backtrace_func);
}

// Benchmark functions
static void BM_libc_backtrace(benchmark::State& state) {
    void* buffer[16];
    for (auto _ : state) {
        func1(buffer, backtrace);
        benchmark::DoNotOptimize(buffer);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_libc_backtrace);

static void BM_gcc_backtrace(benchmark::State& state) {
    void* buffer[16];
    for (auto _ : state) {
        func1(buffer, gccBacktrace);
        benchmark::DoNotOptimize(buffer);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_gcc_backtrace);

static void BM_asm_backtrace(benchmark::State& state) {
    void* buffer[16];
    for (auto _ : state) {
        func1(buffer, asmBacktrace);
        benchmark::DoNotOptimize(buffer);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_asm_backtrace);

#if __cplusplus >= 202302L
static void BM_stacktrace_backtrace(benchmark::State& state) {
    for (auto _ : state) {
        func1(buffer, stacktraceBacktrace);
        benchmark::DoNotOptimize(buffer);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_stacktrace_backtrace);
#endif

// unw_backtrace libunwind.so 不测了
// static void BM_unw_backtrace(benchmark::State& state) {
//     void* buffer[16];
//     for (auto _ : state) {
//         func1(buffer, unw_backtrace);
//         benchmark::DoNotOptimize(buffer);
//     }
//     state.SetItemsProcessed(state.iterations());
// }
// BENCHMARK(BM_unw_backtrace);

// 多线程测试
static void BM_libc_backtrace_mt(benchmark::State& state) {
    if (state.thread_index() == 0) {
        // 主线程初始化
    }
    
    void* buffer[16];
    for (auto _ : state) {
        func1(buffer, backtrace);
        benchmark::DoNotOptimize(buffer);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_libc_backtrace_mt)->Threads(4)->Threads(8)->Threads(16);

static void BM_gcc_backtrace_mt(benchmark::State& state) {
    void* buffer[16];
    for (auto _ : state) {
        func1(buffer, gccBacktrace);
        benchmark::DoNotOptimize(buffer);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_gcc_backtrace_mt)->Threads(4)->Threads(8)->Threads(16);

static void BM_asm_backtrace_mt(benchmark::State& state) {
    void* buffer[16];
    for (auto _ : state) {
        func1(buffer, asmBacktrace);
        benchmark::DoNotOptimize(buffer);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_asm_backtrace_mt)->Threads(4)->Threads(8)->Threads(16);

#if __cplusplus >= 202302L
static void BM_stacktrace_backtrace_mt(benchmark::State& state) {
    for (auto _ : state) {
        func1(buffer, stacktraceBacktrace);
        benchmark::DoNotOptimize(buffer);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_stacktrace_backtrace_mt)->Threads(4)->Threads(8)->Threads(16);
#endif

BENCHMARK_MAIN();

// int main(int argc, char** argv) {
//     print_backtrace_comparison();
    
//     ::benchmark::Initialize(&argc, argv);
//     if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
//     ::benchmark::RunSpecifiedBenchmarks();
//     ::benchmark::Shutdown();
//     return 0;
// }