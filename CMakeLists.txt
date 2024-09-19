#linux: cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=${GCC_PATH}/bin/gcc -DCMAKE_CXX_COMPILER=${GCC_PATH}/bin/g++ ..
cmake_minimum_required(VERSION 3.22)
project(learn_cpp)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(include)

set(gtest_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/googletest)
if(EXISTS ${gtest_SOURCE_DIR})
    message(STATUS "Using local Google Test from ${gtest_SOURCE_DIR}")
    add_subdirectory(${gtest_SOURCE_DIR})
    set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include)
    set(COMMON_GTEST_LIBS gtest gtest_main)
else()
    # set(CMAKE_PREFIX_PATH "third_party/googletest/build/install" ${CMAKE_PREFIX_PATH})
    find_package(GTest REQUIRED)
    if(NOT GTEST_FOUND)
        message(FATAL_ERROR "Google Test not found")
    endif ()
    set(COMMON_GTEST_LIBS ${GTEST_LIBRARIES} pthread)
endif()

# 设置全局编译选项
if(MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        add_compile_options(/Od /Zi /Zc:__cplusplus /Zc:preprocessor)
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        add_compile_options(-g -O0)
    endif()
endif()

add_subdirectory(demo)