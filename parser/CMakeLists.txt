cmake_minimum_required(VERSION 3.10)

include_directories(${PATH_ROOT})

add_library(parser
    ./parser_fsm.cpp
)