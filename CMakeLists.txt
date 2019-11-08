cmake_minimum_required(VERSION 3.6)

project(bun)

set(BLIB_HEADER include/blib)
set(THIRD_PARTY_HEADER
    include/
    include/third_party
    include/third_party/hdr
    include/third_party/json
    include/third_party/msgpack
    include/third_party/rapidjson
    include/third_party/spdlog
    include/third_party/sqlite
    include/third_party/unqlite
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(SOURCE_FILES
        src/sqlite/sqlite3.c
#        src/BoostHeaders.cpp
        src/Bun.cpp
#        src/Generate.cpp
#        src/QueryExample.cpp
        include/blib/bun/bun.hpp
        include/blib/utils/MD5.hpp
        include/blib/utils/Singleton.hpp
)


include(FindBoost)
find_package( Boost 1.65 COMPONENTS system filesystem program_options REQUIRED )

include(FindSoci)
find_package( Soci 3.2 COMPONENTS sqlite3 REQUIRED )

include_directories( ${Boost_INCLUDE_DIR} ${SOCI_INCLUDE_DIR} )
if(${SOCI_FOUND})
  link_directories(${SOCI_LIBRARY} ${SOCI_sqlite3_PLUGIN})
else()
  message(WARNING "SOCI not found")
endif()

add_definitions(-DBUN_SQLITE -DFMT_HEADER_ONLY)

include_directories(include)
include_directories(${BLIB_HEADER})
include_directories(${THIRD_PARTY_HEADER})
include_directories(${INCLUDE_DIR})
include_directories(${SOCI_INCLUDE})

add_library(libsoci_sqlite3 STATIC IMPORTED)
add_library(libsoci_core STATIC IMPORTED include/blib/bun/QueryHelper.hpp)

find_library (SOCI_LIB NAME libsoci_sqlite3 HINTS ${SOCI_LIBRARY})
add_executable(bun ${SOURCE_FILES})
target_link_libraries(bun libsoci_sqlite3.a libsoci_core.a dl pthread)
