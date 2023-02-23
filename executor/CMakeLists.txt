cmake_minimum_required(VERSION 3.10)

include_directories(${PATH_ROOT})

add_library(executor
    ./executor_dispatcher.cpp
    ./executor_sql.cpp
)