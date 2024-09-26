project(test_stl)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../stl/include
    )
create_executable(dummy_array_iterator SOURCE_FILES src/test_dummy_array_iterator.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(test_function SOURCE_FILES src/test_function.cpp LINK_LIBS ${COMMON_GTEST_LIBS})
create_executable(test_uniqueptr SOURCE_FILES src/test_uniqueptr.cpp LINK_LIBS ${COMMON_GTEST_LIBS} CXX_STANDARD 20)
