cmake_minimum_required(VERSION 3.6)

project(bun)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_CXX_FLAGS_DISTRIBUTION "-E")
SET(CMAKE_C_FLAGS_DISTRIBUTION "-E")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -E")
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
        include/blib/bun/DbBackend.hpp)

include_directories(include)
include_directories($ENV{BOOST_HEADER})
include_directories($ENV{BLIB_HEADER})
include_directories($ENV{THIRD_PARTY_HEADER})
include_directories($ENV{INCLUDE_DIR})
include_directories($ENV{SOCI_INCLUDE})

link_directories($ENV{LIB_DIR})

message("Print Dir: " $ENV{LIB_DIR})
add_library(libsoci_sqlite3 STATIC IMPORTED)
add_library(libsoci_core STATIC IMPORTED)

#set(LIBS ${LIBS} $ENV{LIB_DIR})
find_library (SOCI_LIB NAME libsoci_sqlite3 HINTS $ENV{LIB_DIR})
message("Lib = $ENV{LD_LIBRARY_PATH}")
add_executable(bun ${SOURCE_FILES})
target_link_libraries(bun $ENV{LIB_DIR}/libsoci_sqlite3.a $ENV{LIB_DIR}/libsoci_core.a)
