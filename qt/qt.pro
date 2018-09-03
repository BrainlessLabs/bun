TEMPLATE = app
CONFIG += console c++14
CONFIG -= app_bundle
CONFIG -= qt

#SUBDIRS += \
    #bun.pro

PARENT_DIR = $$dirname(PWD)
windows{
BOOST_HEADERS = $$getenv(BOOST_DIR)
SOCI_HEADERS = $$getenv(SOCI_DIR)/include
SOCI_LIBS = $$getenv(SOCI_DIR)/lib64
}
linux{
BOOST_HEADERS = /mnt/hgfs/code/boost_1_64_0/
SOCI_HEADERS = /usr/local/include/
SOCI_LIBS = /usr/local/lib64/
}

message(pwd: $$PARENT_DIR)
message(soci_h: $$SOCI_HEADERS)
message(boost_h: $$BOOST_HEADERS)

DEFINES += \
BOOST_ALL_NO_LIB \
FMT_HEADER_ONLY \
BUN_SQLITE \
BOOST_SYSTEM_NO_LIB \
BOOST_SYSTEM_NO_DEPRECATED \
BOOST_ERROR_CODE_HEADER_ONLY

message(defines: $$DEFINES)

SOURCES += \
$$PARENT_DIR/src/Generate.cpp \
$$PARENT_DIR/include/third_party/spdlog/fmt/bundled/ostream.cc \
$$PARENT_DIR/src/sqlite/sqlite3.c \
$$PARENT_DIR/src/unqlite.c

message(sources: $$SOURCES)

HEADERS += \
$$PARENT_DIR/include/third_party/sqlite/sqlite3.h \
$$PARENT_DIR/include/third_party/sqlite/sqlite3ext.h \
$$PARENT_DIR/include/third_party/unqlite/unqlite.h \
$$PARENT_DIR/include/blib/bun/bun.hpp \
$$PARENT_DIR/include/blib/bun/impl/orm.hpp \
$$PARENT_DIR/include/blib/bun/impl/CppTypeToSQLString.hpp \
$$PARENT_DIR/include/blib/bun/impl/DbBackend.hpp \
$$PARENT_DIR/include/blib/bun/impl/DbLogger.hpp \
$$PARENT_DIR/include/blib/bun/impl/endian.hpp \
$$PARENT_DIR/include/blib/bun/impl/kv.hpp \
$$PARENT_DIR/include/blib/bun/impl/SimpleOID.hpp \
$$PARENT_DIR/include/blib/utils/debug.hpp \
$$PARENT_DIR/include/blib/utils/MD5.hpp \
$$PARENT_DIR/include/blib/utils/Singleton.hpp \
$$PARENT_DIR/include/blib/bun/impl/iterators.hpp

message(headers: $$HEADERS)

INCLUDEPATH += \
$$PARENT_DIR/include/ \
$$PARENT_DIR/include/third_party/ \
$$PARENT_DIR/include/third_party/sqlite/ \
$$PARENT_DIR/include/third_party/fmt/ \
$$PARENT_DIR/include/blib/ \
$$BOOST_HEADERS/ \
$$PARENT_DIR/include/third_party/unqlite/ \
$$SOCI_HEADERS

message(includepath: $$INCLUDEPATH)

LIBPATH += \
$$SOCI_LIBS

windows{
LIBS += \
libsoci_core_4_0.lib \
libsoci_empty_4_0.lib \
libsoci_sqlite3_4_0.lib
}
linux{
LIBS += \
libsoci_core_4_0 \
libsoci_empty_4_0 \
libsoci_sqlite3_4_0
}


DISTFILES += \
qt.pro.unix \
../.gitignore

