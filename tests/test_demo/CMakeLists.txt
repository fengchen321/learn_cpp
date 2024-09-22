project(test_demo)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../demo/include
    )
# Create executables with optional library linking
create_executable(code_performance SOURCE_FILES src/code_performance.cpp)
create_executable(macro_usage SOURCE_FILES src/test_macro_usage.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(template_SFINAE SOURCE_FILES src/test_template_SFINAE.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(print SOURCE_FILES src/test_template_print.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(common_type SOURCE_FILES src/test_template_common_type.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(template_type_conversion SOURCE_FILES src/test_template_type_conversion.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(template_compile_for_loop SOURCE_FILES src/test_template_compile_for_loop.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(format_print SOURCE_FILES src/format_print.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
create_executable(random_generation SOURCE_FILES src/test_random_generation.cpp LINK_LIBS ${COMMON_GTEST_LIBS}) 
create_executable(value_category SOURCE_FILES src/value_category.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(template_compile_reflect SOURCE_FILES src/test_template_compile_reflect.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
if(NOT MSVC)
    create_executable(json_parser SOURCE_FILES src/test_json_parser.cpp LINK_LIBS ${COMMON_GTEST_LIBS} demo_lib)
endif()