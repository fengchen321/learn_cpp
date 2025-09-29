include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../stl/include
    )
create_executable(dummy_array_iterator 
    SOURCE_FILES src/test_dummy_array_iterator.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(test_function 
    SOURCE_FILES src/test_function.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(test_uniqueptr 
    SOURCE_FILES src/test_uniqueptr.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} 
    CXX_STANDARD 20)

create_executable(test_memory_resource 
    SOURCE_FILES src/test_memory_resource.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS} 
    CXX_STANDARD 20)

create_executable(test_array 
    SOURCE_FILES src/test_array.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(test_vector 
    SOURCE_FILES src/test_vector.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(test_list 
    SOURCE_FILES src/test_list.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

create_executable(test_optional 
    SOURCE_FILES src/test_optional.cpp 
    LINK_LIBS ${COMMON_GTEST_LIBS})

