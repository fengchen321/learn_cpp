include_directories(
    include)

add_library(stl_lib 
    src/dummy_array_iterator.cpp
)

target_include_directories(stl_lib PUBLIC include)
