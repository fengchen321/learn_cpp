#include <iostream>
#include <sstream>
#include "ScopeProfiler.h"
#include <benchmark/benchmark.h>

const std::string strTest = "12345678901234567890";  // 20字节
const int count = 1000000;

void testStringComplete() {
    const int REPEAT_COUNT = 3;
    
    for (int run = 0; run < REPEAT_COUNT; run++) {
        // 测试1: 普通字符串拼接（无reserve）
        {
            std::string str;
            ScopeProfiler _("string += (no reserve)");
            for (size_t i = 0; i < count; i++) {
                str += strTest;
            }
        }
        // 测试2: 预分配内存的字符串拼接
        {
            std::string str;
            ScopeProfiler _("string += (with reserve)");
            str.reserve(count * strTest.size());
            for (size_t i = 0; i < count; i++) {
                str += strTest;
            }
        }
        // 后面的测试受益于 CPU cache 预热。导致append看起来比+快
        // 测试3: 使用append方法
        {
            std::string str;
            ScopeProfiler _("string::append (with reserve)");
            str.reserve(count * strTest.size());
            for (size_t i = 0; i < count; i++) {
                str.append(strTest);
            }
        }
        
        // 测试4: 普通stringstream
        {
            std::stringstream ss;
            ScopeProfiler _("stringstream (normal)");
            for (size_t i = 0; i < count; i++) {
                ss << strTest;
            }
            std::string result = ss.str();
        }
        
        // 测试5: 预分配buffer的stringstream
        {
            std::stringstream ss;
            std::string buffer;
            buffer.reserve(count * strTest.size() * 2);  // 预分配足够大的buffer
            ss.rdbuf()->pubsetbuf(&buffer[0], buffer.capacity());
            ScopeProfiler _("stringstream (pre-buffer)");
            for (size_t i = 0; i < count; i++) {
                ss << strTest;
            }
            std::string result = ss.str();
        }
    } 
    printScopeProfiler();
}

static void BM_StringPlusNoReserve(benchmark::State& state) {
    const int count = state.range(0);
    
    for (auto _ : state) {
        std::string str;
        for (int i = 0; i < count; i++) {
            str += strTest;
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_StringPlusNoReserve)->Arg(1000000);

static void BM_StringPlusWithReserve(benchmark::State& state) {
    const int count = state.range(0);
    
    for (auto _ : state) {
        std::string str;
        str.reserve(count * strTest.size());
        for (int i = 0; i < count; i++) {
            str += strTest;
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_StringPlusWithReserve)->Arg(1000000);

static void BM_StringAppendWithReserve(benchmark::State& state) {
    const int count = state.range(0);
    
    for (auto _ : state) {
        std::string str;
        str.reserve(count * strTest.size());
        for (int i = 0; i < count; i++) {
            str.append(strTest);
        }
        benchmark::DoNotOptimize(str);
    }
}
BENCHMARK(BM_StringAppendWithReserve)->Arg(1000000);

static void BM_StringStream(benchmark::State& state) {
    const int count = state.range(0);
    
    for (auto _ : state) {
        std::stringstream ss;
        for (int i = 0; i < count; i++) {
            ss << strTest;
        }
        std::string result = ss.str();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringStream)->Arg(1000000);

static void BM_StringStreamPreBuffer(benchmark::State& state) {
    const int count = state.range(0);
    
    for (auto _ : state) {
        std::ostringstream oss;
        std::string buffer;
        buffer.reserve(count * strTest.size() * 2);
        oss.rdbuf()->pubsetbuf(&buffer[0], buffer.capacity());
        for (int i = 0; i < count; i++) {
            oss << strTest;
        }
        std::string result = oss.str();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StringStreamPreBuffer)->Arg(1000000);

int main(int argc, char** argv) {
    std::cout << "=== 完整字符串拼接性能对比测试 ===" << std::endl;
    std::cout << "测试数据: " << count << " 次拼接，每次 " << strTest.size() << " 字节" << std::endl;
    std::cout << "总数据量: " << (count * strTest.size() / 1024 / 1024) << " MB" << std::endl;
    
    testStringComplete();
    
    std::cout << "\n=== Google Benchmark 详细测试 ===" << std::endl;
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
    
    return 0;
}