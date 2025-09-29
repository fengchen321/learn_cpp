include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../demo/include
    )
# add_compile_options() 添加编译选项，可直接man gcc查看相关选项
# Create executables with optional library linking

create_executable(macro_usage 
    SOURCE_FILES src/test_macro_usage.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS}
    CXX_STANDARD 20)

create_executable(template_SFINAE 
    SOURCE_FILES src/test_template_SFINAE.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} 
    CXX_STANDARD 20)

create_executable(print 
    SOURCE_FILES src/test_template_print.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(common_type 
    SOURCE_FILES src/test_template_common_type.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS}
     CXX_STANDARD 20)

create_executable(template_type_conversion 
    SOURCE_FILES src/test_template_type_conversion.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS}
    CXX_STANDARD 20)

create_executable(template_compile_for_loop
    SOURCE_FILES src/test_template_compile_for_loop.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(format_print
    SOURCE_FILES src/format_print.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} 
    CXX_STANDARD 20)

create_executable(random_generation 
    SOURCE_FILES src/test_random_generation.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS}) 

create_executable(value_category 
    SOURCE_FILES src/value_category.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(template_compile_reflect 
    SOURCE_FILES src/test_template_compile_reflect.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} 
    CXX_STANDARD 20)

if(NOT MSVC)
    create_executable(json_parser 
        SOURCE_FILES src/test_json_parser.cpp 
        LINK_LIBS ${COMMON_GTEST_LIBS} demo_lib)
endif()

create_executable(test_ptr_lifecycle 
    SOURCE_FILES src/test_ptr_lifecycle.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(test_template_map_fold 
    SOURCE_FILES src/test_template_map_fold.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(hexdump_tool 
    SOURCE_FILES src/hexdump_tool.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(api_design 
    SOURCE_FILES src/api_design.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS}) #CXX_STANDARD 20)

create_executable(design_pattern 
    SOURCE_FILES src/design_pattern.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS}) #CXX_STANDARD 20)

create_executable(test_thread 
    SOURCE_FILES src/test_thread.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} 
    CXX_STANDARD 20)

create_executable(test_move 
    SOURCE_FILES src/test_move.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(test_undef 
    SOURCE_FILES src/test_undef.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} 
    CXX_STANDARD 20)

create_executable(test_ostream 
    SOURCE_FILES src/test_ostream.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} demo_lib)

create_executable(test_input_password 
    SOURCE_FILES src/test_input_password.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} demo_lib)
