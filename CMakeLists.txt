cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

project(sql)

set(PATH_ROOT $(CMAKE_CURRENT_SOURCE_DIR))

include_directories(${PATH_ROOT})

add_subdirectory(parser)
add_subdirectory(executor)
add_subdirectory(common)
add_subdirectory(def)
add_subdirectory(3rd/cpp-linenoise)

add_executable(sql
    ./main.cpp
)

target_link_libraries(sql
    app
)