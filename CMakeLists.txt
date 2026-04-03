#linux: cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=${GCC_PATH}/bin/gcc -DCMAKE_CXX_COMPILER=${GCC_PATH}/bin/g++ ..
cmake_minimum_required(VERSION 3.22)
project(learn_cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_VERBOSE_MAKEFILE ON)

include_directories(include)
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

# ---- Google Test Setup ----
set(gtest_SOURCE_DIR ${CMAKE_SOURCE_DIR}/third_party/googletest)
if(EXISTS ${gtest_SOURCE_DIR})
    message(STATUS "Using local Google Test from ${gtest_SOURCE_DIR}")
    add_subdirectory(${gtest_SOURCE_DIR} ${CMAKE_BINARY_DIR}/googletest)
    set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include)
    set(COMMON_GTEST_LIBS gtest gtest_main)
else()
    find_package(GTest REQUIRED)
    if(NOT GTEST_FOUND)
        message(FATAL_ERROR "Google Test not found")
    endif ()
    set(COMMON_GTEST_LIBS ${GTEST_LIBRARIES} pthread)
endif()

include_directories(${GTEST_INCLUDE_DIRS})

# Define a macro to create an executable with optional C++ standard and libraries
function(create_executable name)
    set(options)
    set(oneValueArgs CXX_STANDARD)
    set(multiValueArgs SOURCE_FILES LINK_LIBS)
    cmake_parse_arguments(PARSED
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN})

    add_executable(${name} ${PARSED_SOURCE_FILES})

    if(PARSED_CXX_STANDARD)
        set_target_properties(${name} PROPERTIES
                CXX_STANDARD ${PARSED_CXX_STANDARD}
                CXX_STANDARD_REQUIRED ON
        )
    endif()

    if(PARSED_LINK_LIBS)
        target_link_libraries(${name} ${PARSED_LINK_LIBS})
    endif()
    add_test(NAME ${name} COMMAND ${name})
endfunction()

enable_testing()

# ---- Modules ----
add_subdirectory(demo)
add_subdirectory(stl)
add_subdirectory(projects/perf_duration_trace)
add_subdirectory(test_performance)