cmake_minimum_required(VERSION 3.10)

link_directories(${PATH_ROOT}/parser/)

include_directories(${PATH_ROOT}/3rd/cpp-linenoise)
include_directories(${PATH_ROOT})
include_directories(${PATH_ROOT}/common)

add_library(app
    ./sql_app.cpp
    ./sql_app_util.cpp
)

target_link_libraries(app
    parser
    executor
    linenoise
)