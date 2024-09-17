cmake_minimum_required(VERSION 3.22)
project(demo)

find_package(GTest REQUIRED)
include_directories(
    include
    ${GTEST_INCLUDE_DIRS})

set(COMMON_GTEST_LIBS ${GTEST_LIBRARIES} pthread)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define a macro to create an executable with optional C++20 standard and libraries
function(create_executable name)
    # Define argument parsing
    set(options)
    set(oneValueArgs CXX_STANDARD)
    set(multiValueArgs SOURCE_FILES LINK_LIBS)
    cmake_parse_arguments(PARSED
            "${options}"
            "${oneValueArgs}"
            "${multiValueArgs}"
            ${ARGN})

    # Add the executable
    add_executable(${name} ${PARSED_SOURCE_FILES})

    # Optionally set C++ standard if specified
    if(PARSED_CXX_STANDARD)
        set_target_properties(${name} PROPERTIES
                CXX_STANDARD ${PARSED_CXX_STANDARD}
                CXX_STANDARD_REQUIRED ON
        )
    endif()

    # Link libraries if specified
    if(PARSED_LINK_LIBS)
        target_link_libraries(${name} ${PARSED_LINK_LIBS})
    endif()
    # target_link_libraries(${name} ${COMMON_GTEST_LIBS})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# Create executables with optional library linking
# ctest -R ${name} -V / --verbose
# ./demo/format_print --gtest_filter=FormatPrint.MiniLogTest
create_executable(code_performance SOURCE_FILES src/code_performance.cpp)
create_executable(macro_usage SOURCE_FILES src/macro_usage.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(template_SFINAE SOURCE_FILES src/template_SFINAE.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(print SOURCE_FILES src/template_print.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(common_type SOURCE_FILES src/template_common_type.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(dummy_array_iterator SOURCE_FILES src/dummy_array_iterator.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(template_type_conversion SOURCE_FILES src/template_type_conversion.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(template_compile_for_loop SOURCE_FILES src/template_compile_for_loop.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(format_print SOURCE_FILES src/format_print.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(random_generation SOURCE_FILES src/random_generation.cpp LINK_LIBS ${COMMON_GTEST_LIBS}) 
create_executable(value_category SOURCE_FILES src/value_category.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(template_compile_reflect SOURCE_FILES src/template_compile_reflect.cpp LINK_LIBS ${COMMON_GTEST_LIBS})# CXX_STANDARD 20)
