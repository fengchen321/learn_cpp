project(demo)

include_directories(
    include)

add_library(demo_lib
    src/json_parser.cpp
    src/ostream.cpp
)

target_include_directories(demo_lib PUBLIC include)