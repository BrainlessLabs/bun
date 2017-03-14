cmake_minimum_required(VERSION 3.6)

project(bun)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/sqlite/sqlite3.c
        src/BoostHeaders.cpp
#        src/Bun.cpp
#        src/Generate.cpp
#        src/QueryExample.cpp
        include/blib/bun/bun.hpp
        include/blib/bun/CppTypeToSQLString.hpp
        include/blib/bun/DbLogger.hpp
        include/blib/bun/SimpleOID.hpp
        include/blib/utils/MD5.hpp
        include/blib/utils/Singleton.hpp
        include/blib/utils/TypeUtils.hpp
        include/blib/bun/PRef.hpp
        include/blib/bun/BunHelper.hpp
        include/blib/bun/DBBackend.hpp)

#include_directories(include)
include_directories($ENV{BOOST_HEADER})
include_directories($ENV{BLIB_HEADER})
include_directories($ENV{THIRD_PARTY_HEADER})
include_directories($ENV{INCLUDE_DIR})
include_directories($ENV{SOCI_INCLUDE})

link_directories($ENV{LIB_DIR})

message(Print Dir:)
add_library(libsoci_sqlite3.a STATIC IMPORTED)

add_executable(bun ${SOURCE_FILES})
target_link_libraries(bun libsoci_sqlite3)