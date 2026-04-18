cmake_minimum_required(VERSION 3.13.4)

add_library(calculator_core SHARED
    src/node.cpp
    src/scanner.cpp
    src/parser.cpp
    src/ast_builder.cpp
    src/symbol_table.cpp
    src/storage.cpp
    src/env.cpp
    src/func_table.cpp
    src/exception.cpp
    src/commandParser.cpp
)

target_include_directories(calculator_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(calculator_core PRIVATE
        -Wall -Wextra -Wpedantic
    )
    # For backtrace symbol resolution
    target_link_libraries(calculator_core PRIVATE dl)
    target_link_options(calculator_core PRIVATE -rdynamic)
endif()

add_executable(calculator src/calculator.cpp)
target_link_libraries(calculator PRIVATE calculator_core)

create_executable(test_calculator_parser
    SOURCE_FILES tests/src/test_parser.cpp
    LINK_LIBS calculator_core ${COMMON_GTEST_LIBS})

create_executable(test_calculator_runtime
    SOURCE_FILES tests/src/test_runtime_state.cpp
    LINK_LIBS calculator_core ${COMMON_GTEST_LIBS})
