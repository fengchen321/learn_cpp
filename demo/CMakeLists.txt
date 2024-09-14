cmake_minimum_required(VERSION 3.23)
project(demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include_directories(include)
# Define a macro to create an executable with optional C++20 standard
function(create_executable name)
    # Define argument parsing
    set(options)
    set(oneValueArgs CXX_STANDARD)
    set(multiValueArgs SOURCE_FILES)
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
endfunction()

# Create executables with default C++17
create_executable(code_performance SOURCE_FILES src/code_performance.cpp)
create_executable(macro_usage SOURCE_FILES src/macro_usage.cpp CXX_STANDARD 20)
create_executable(template_SFINAE SOURCE_FILES src/template_SFINAE.cpp CXX_STANDARD 20)
create_executable(print SOURCE_FILES src/template_print.cpp)
create_executable(common_type SOURCE_FILES src/template_common_type.cpp  CXX_STANDARD 20)
create_executable(dummy_array_iterator SOURCE_FILES src/dummy_array_iterator.cpp)
