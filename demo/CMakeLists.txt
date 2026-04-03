include_directories(
    include)

add_library(demo_lib
    src/json_parser.cpp
    src/ostream.cpp
    src/input_password.cpp
)

target_include_directories(demo_lib PUBLIC include)

# Standalone single-file tools (no library dependency)
add_executable(clock_perf_benchmark tools/clock_perf_benchmark.cpp)
target_link_libraries(clock_perf_benchmark PRIVATE rt)
target_compile_options(clock_perf_benchmark PRIVATE -O3 -ggdb)

add_executable(clock_info_query tools/clock_info_query.cpp)

add_subdirectory(tests)
