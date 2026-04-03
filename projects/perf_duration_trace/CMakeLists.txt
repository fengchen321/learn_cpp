cmake_minimum_required(VERSION 3.13.4)

find_package(benchmark REQUIRED)

# ---- Tests ----

add_executable(perf_duration_trace_test
    tests/perf_duration_trace_test.cpp
    tests/perf_duration_trace_test_site_a.cpp
    tests/perf_duration_trace_test_site_b.cpp)
target_include_directories(perf_duration_trace_test PRIVATE include)
target_compile_definitions(perf_duration_trace_test PRIVATE PERF_ENABLED=1)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(perf_duration_trace_test PRIVATE
        -O2 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wstrict-aliasing=2)
endif()
target_link_libraries(perf_duration_trace_test PRIVATE pthread)
add_test(NAME perf_duration_trace_test COMMAND perf_duration_trace_test)
set_tests_properties(perf_duration_trace_test PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(perf_duration_trace_disabled_test
    tests/perf_duration_trace_disabled_test.cpp)
target_include_directories(perf_duration_trace_disabled_test PRIVATE include)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(perf_duration_trace_disabled_test PRIVATE
        -O2 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wstrict-aliasing=2)
endif()
target_link_libraries(perf_duration_trace_disabled_test PRIVATE pthread)
add_test(NAME perf_duration_trace_disabled_test COMMAND perf_duration_trace_disabled_test)

add_executable(perf_duration_trace_analyze_smoke
    tests/perf_duration_trace_analyze_smoke.cpp)
target_include_directories(perf_duration_trace_analyze_smoke PRIVATE include)
target_compile_definitions(perf_duration_trace_analyze_smoke PRIVATE PERF_ENABLED=1)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(perf_duration_trace_analyze_smoke PRIVATE
        -O2 -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Wstrict-aliasing=2)
endif()
target_link_libraries(perf_duration_trace_analyze_smoke PRIVATE pthread)
add_test(NAME perf_duration_trace_analyze_smoke COMMAND perf_duration_trace_analyze_smoke)
set_tests_properties(perf_duration_trace_analyze_smoke PROPERTIES
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# ---- Benchmarks ----

function(perf_duration_trace_bench name source)
    add_executable(${name} ${source})
    target_include_directories(${name} PRIVATE include)
    target_link_libraries(${name} PRIVATE benchmark::benchmark pthread)
    target_compile_options(${name} PRIVATE -O3)
endfunction()

perf_duration_trace_bench(perf_duration_trace_benchmark
    benchmarks/perf_duration_trace_benchmark.cpp)
target_compile_definitions(perf_duration_trace_benchmark PRIVATE PERF_ENABLED=1)

perf_duration_trace_bench(perf_duration_trace_disabled_benchmark
    benchmarks/perf_duration_trace_disabled_benchmark.cpp)
