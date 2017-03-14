cmake_minimum_required(VERSION 3.6)
project(bun)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/sqlite/sqlite3.c
        src/BoostHeaders.cpp
        src/Bun.cpp
        src/Generate.cpp
        src/QueryExample.cpp)

include_directories(include)

add_executable(bun ${SOURCE_FILES})