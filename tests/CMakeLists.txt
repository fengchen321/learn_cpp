set(gtest_SOURCE_DIR ${CMAKE_SOURCE_DIR}/../third_party/googletest)
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

include_directories(
    include
    ${GTEST_INCLUDE_DIRS})
    
add_subdirectory(test_demo)
add_subdirectory(test_stl)
