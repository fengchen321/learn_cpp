#include <iostream>
#include <chrono>
#include "ScopeProfiler.h"
#include <benchmark/benchmark.h>

class DynamicInterface {
public:
    virtual void tick(uint64_t n) = 0;
    virtual uint64_t getValue() = 0;
    virtual ~DynamicInterface() = default;
};

class DynamicImplementation : public DynamicInterface {
public:
    DynamicImplementation() : counter{0} {}
    void tick(uint64_t n) override { counter += n; }
    uint64_t getValue() override { return counter; }
private:
    uint64_t counter;
};

template <typename Implementation>
class CRTPInterface {
public:
    void tick(uint64_t n) { impl().tick(n); }
    uint64_t getValue() { return impl().getValue(); }
    virtual ~CRTPInterface() = default;
private:
    Implementation& impl() {
        return *static_cast<Implementation*>(this);
    }
};

class CRTPImplementation : public CRTPInterface<CRTPImplementation> {
public:
    CRTPImplementation() : counter{0} {}
    void tick(uint64_t n) { counter += n; }
    uint64_t getValue() { return counter; }
private:
    uint64_t counter;
};

void test_ScopeProfiler() {
    const unsigned N = 10'000'000;
    
    // 重复多次以获得有意义的统计
    const int REPEAT_COUNT = 5;
    
    for (int run = 0; run < REPEAT_COUNT; run++) {
        {
            ScopeProfiler _("DynamicInterface");
            DynamicImplementation dynObj;
            DynamicInterface* dynPtr = &dynObj;
            for (unsigned i = 0; i < N; ++i) {
                dynPtr->tick(i);
            }
            benchmark::DoNotOptimize(dynPtr->getValue());
        }
        
        {
            ScopeProfiler _("CRTPInterface");
            CRTPImplementation crtpObj;
            CRTPInterface<CRTPImplementation>* crtpPtr = &crtpObj;
            for (unsigned i = 0; i < N; ++i) {
                crtpPtr->tick(i);
            }
            benchmark::DoNotOptimize(crtpPtr->getValue());
        }
        
        {
            ScopeProfiler _("DirectCall");
            CRTPImplementation directObj;
            for (unsigned i = 0; i < N; ++i) {
                directObj.tick(i);
            }
            benchmark::DoNotOptimize(directObj.getValue());
        }
    }
    
    printScopeProfiler();
}


static void BM_DynamicInterface(benchmark::State& state) {
    const unsigned N = state.range(0);
    
    for (auto _ : state) {
        DynamicImplementation dynObj;
        DynamicInterface* dynPtr = &dynObj;
        for (unsigned i = 0; i < N; ++i) {
            dynPtr->tick(i);
        }
        benchmark::DoNotOptimize(dynPtr->getValue());
    }
}
BENCHMARK(BM_DynamicInterface)->Arg(10'000'000)->Unit(benchmark::kMicrosecond);

static void BM_CRTPInterface(benchmark::State& state) {
    const unsigned N = state.range(0);
    
    for (auto _ : state) {
        CRTPImplementation crtpObj;
        CRTPInterface<CRTPImplementation>* crtpPtr = &crtpObj;
        for (unsigned i = 0; i < N; ++i) {
            crtpPtr->tick(i);
        }
        benchmark::DoNotOptimize(crtpPtr->getValue());
    }
}
BENCHMARK(BM_CRTPInterface)->Arg(10'000'000)->Unit(benchmark::kMicrosecond);

static void BM_DirectCall(benchmark::State& state) {
    const unsigned N = state.range(0);
    
    for (auto _ : state) {
        CRTPImplementation directObj;
        for (unsigned i = 0; i < N; ++i) {
            directObj.tick(i);
        }
        benchmark::DoNotOptimize(directObj.getValue());
    }
}
BENCHMARK(BM_DirectCall)->Arg(10'000'000)->Unit(benchmark::kMicrosecond);

int main(int argc, char** argv) {
    std::cout << "=== ScopeProfiler 测试结果 ===" << std::endl;
    test_ScopeProfiler();
    
    std::cout << "\n=== Google Benchmark 测试结果 ===" << std::endl;
    ::benchmark::Initialize(&argc, argv);
    if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
    ::benchmark::RunSpecifiedBenchmarks();
    ::benchmark::Shutdown();
    
    return 0;
}

// BENCHMARK_MAIN();