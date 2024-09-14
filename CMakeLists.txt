#linux: cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=${GCC_PATH}/bin/gcc -DCMAKE_CXX_COMPILER=${GCC_PATH}/bin/g++ ..
cmake_minimum_required(VERSION 3.22)
project(learn_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(include)

# 设置全局编译选项
if(MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        add_compile_options(/Od /Zi )
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        add_compile_options(-g -O0)
    endif()
endif()

add_subdirectory(demo)