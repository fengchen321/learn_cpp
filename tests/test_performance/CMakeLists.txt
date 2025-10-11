cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-g -fno-omit-frame-pointer)

find_package(benchmark REQUIRED)

function(performance_test name)
    set(options)
    set(oneValueArgs CXX_STANDARD)
    set(multiValueArgs SOURCE_FILES LINK_LIBS COMPILE_OPTIONS)
    cmake_parse_arguments(PARSED
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN})

    add_executable(${name} ${PARSED_SOURCE_FILES})

    target_include_directories(${name} PRIVATE include)

    if(PARSED_CXX_STANDARD)
        set_target_properties(${name} PROPERTIES
                CXX_STANDARD ${PARSED_CXX_STANDARD}
                CXX_STANDARD_REQUIRED ON
        )
    endif()

    target_link_libraries(${name} PRIVATE 
        benchmark::benchmark
        ${PARSED_LINK_LIBS}
    )

    if(PARSED_COMPILE_OPTIONS)
        target_compile_options(${name} PRIVATE ${PARSED_COMPILE_OPTIONS})
    endif()

    add_test(NAME ${name} COMMAND ${name})
endfunction()

performance_test(vector_traverse_performance 
    SOURCE_FILES src/vector_traverse_performance.cpp)

performance_test(CRTP_performance 
    SOURCE_FILES src/CRTP_performance.cpp
    COMPILE_OPTIONS -O3)

performance_test(string_benchmark 
    SOURCE_FILES src/string_benchmark.cpp
    COMPILE_OPTIONS -O3)

performance_test(kernel_map_benchmark 
    SOURCE_FILES 
        src/kernel_map_benchmark.cpp
        src/kernel_map_generate.cpp
    LINK_LIBS pthread
    CXX_STANDARD 20
)

performance_test(backtrace_benchmark 
    SOURCE_FILES src/backtrace_benchmark.cpp
    LINK_LIBS pthread dl
    CXX_STANDARD 23
    COMPILE_OPTIONS -O2
)

performance_test(clock_perf_benchmark 
    SOURCE_FILES src/clock_perf_benchmark.cpp
    LINK_LIBS rt
    COMPILE_OPTIONS -O3 -ggdb)

performance_test(clock_info_query 
    SOURCE_FILES src/clock_info_query.cpp)
# ./tests/test_performance/kernel_map_benchmark