#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file Bun.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief The include file for Bun ORM. This file includes all the source
///        to make the class/struct persist in sqlite. Users need to include
///        only this file.
///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <boost/preprocessor.hpp>
#include <third_party/fmt/format.hpp>
#include "blib/bun/BunHelper.hpp"
#include "blib/bun/DbBackend.hpp"
#include "blib/bun/DbLogger.hpp"
#include "blib/bun/CppTypeToSQLString.hpp"

///////////////////////////////////////////////////////////////////////////////
/// @basic Basic Persistance Start
/// @details We will be using the Bun class for all the examples and
///          understanding
///          struct Bun {
///            std::string bun_name,
///            float sugar_quantity;
///            float flour_quantity;
///            float milk_quantity;
///            float yeast_quantity;
///            float butter_quantity;
///            int bun_length;
///          };
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros Start
///////////////////////////////////////////////////////////////////////////////

/// @brief createSchema Helper Macros
/// @details We need to pass only the data members as a tuple to this macro
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
/// @brief Expands the class members for CREATE TABLE
#define EXPAND_CLASS_MEMBERS_createSchemaI(z, n, ELEMS_TUP) BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)) " {}"
#define EXPAND_CLASS_MEMBERS_createSchema(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_CLASS_MEMBERS_createSchemaI, ELEMS_TUP)

/// @brief Expands the class members for CREATE TABLE to get the type info
#define EXPAND_CLASS_TYPE_MEMBERS_createSchemaI(z, n, CLASS_ELEMS_TUP) BOOST_PP_COMMA_IF(n) blib::bun::cppTypeToDbTypeString<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n,1), CLASS_ELEMS_TUP))>()
#define EXPAND_CLASS_TYPE_MEMBERS_createSchema(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), EXPAND_CLASS_TYPE_MEMBERS_createSchemaI, CLASS_ELEMS_TUP)

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////
/// SPECIALIZE_BUN_HELPER Start
#define SPECIALIZE_BUN_HELPER(CLASS_ELEMS_TUP) namespace blib{namespace bun{\
template<>\
struct BunHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
using ClassType = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
inline static void createSchema(){\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const query = "CREATE TABLE IF NOT EXISTS '{}' (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER NOT NULL" \
EXPAND_CLASS_MEMBERS_createSchema(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) \
 ")";\
static std::string const sql = fmt::format(query, class_name, EXPAND_CLASS_TYPE_MEMBERS_createSchema(CLASS_ELEMS_TUP));\
l().info(sql);\
}\
inline static void deleteSchema(){\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const sql = sql = fmt::format("DROP TABLE '{}'", class_name);\
}\
inline static SimpleOID persistObj( T* obj ){\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
SimpleOID oid;\
oid.populateLow();\
return oid;\
}\
};\
}\
}\


/// SPECIALIZE_BUN_HELPER End


///////////////////////////////////////////////////////////////////////////////
/// @basic Basic Persistance End
///////////////////////////////////////////////////////////////////////////////