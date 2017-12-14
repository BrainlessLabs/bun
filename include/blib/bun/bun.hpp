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
#include <memory>
#include <boost/preprocessor.hpp>
#include "blib/bun/PRef.hpp"
#include <third_party/fmt/format.hpp>
#include "blib/bun/PRefHelper.hpp"
#include "blib/bun/DbBackend.hpp"
#include "blib/bun/DbLogger.hpp"
#include "blib/bun/QueryHelper.hpp"
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

/// @brief Make this 0 if no log is needed, else make it 1
#define QUERY_LOG_ON 1
/// @brief Log the query
#define QUERY_LOG(log_string) BOOST_PP_EXPR_IF(QUERY_LOG_ON, l().info(log_string))

/// @brief SOCI ORM Helper Macros
/// @details from_base
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
#define EXPAND_MEMBER_ASSIGNENTS_from_base_I(z, n, ELEMS_TUP) c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP) = v.get<decltype(c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)));
#define EXPAND_MEMBER_ASSIGNENTS_from_base(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_MEMBER_ASSIGNENTS_from_base_I, ELEMS_TUP)

/// @details to_base
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
#define EXPAND_MEMBER_ASSIGNENTS_to_base_I(z, n, ELEMS_TUP) v.set(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)), blib::bun::__private::convertToSOCIType(c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)));
#define EXPAND_MEMBER_ASSIGNENTS_to_base(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_MEMBER_ASSIGNENTS_to_base_I, ELEMS_TUP)

/// @brief createSchema Helper Macros
/// @details We need to pass only the data members as a tuple to this macro
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
/// @brief Expands the class members for CREATE TABLE
#define EXPAND_CLASS_MEMBERS_createSchema_I(z, n, ELEMS_TUP) BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)) " {}"
#define EXPAND_CLASS_MEMBERS_createSchema(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_CLASS_MEMBERS_createSchema_I, ELEMS_TUP)

/// @brief Expands the class members for CREATE TABLE to get the type info
#define EXPAND_CLASS_TYPE_MEMBERS_createSchema_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_COMMA_IF(n) blib::bun::cppTypeToDbTypeString<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n,1), CLASS_ELEMS_TUP))>()
#define EXPAND_CLASS_TYPE_MEMBERS_createSchema(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), EXPAND_CLASS_TYPE_MEMBERS_createSchema_I, CLASS_ELEMS_TUP)

/// @brief persistObj Helper Macros
/// @brief Changes related to INSERT INTO
/// @brief EXPAND_CLASS_TYPE_MEMBERS_persistObj
#define EXPAND_FIELDS_persistObj_I(z, n, ELEMS_TUP) BOOST_PP_IDENTITY(",")() BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_FIELDS_persistObj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_FIELDS_persistObj_I, ELEMS_TUP)

/// @brief Expand the variables for persistObj
#define EXPAND_VARIABLES_persistObj_I(z, n, ELEMS_TUP) BOOST_PP_COMMA_IF(n)obj->BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)
#define EXPAND_VARIABLES_persistObj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_VARIABLES_persistObj_I, ELEMS_TUP)

/// @brief updateObj Helper Macros
/// @brief Changes related to UPDATE table SET
/// @brief Expand the braces for updateObj
#define EXPAND_BRACES_updateObj_I(z, n, ELEMS_TUP) BOOST_PP_EXPR_IF(n, ",")BOOST_STRINGIZE({} = {})
#define EXPAND_BRACES_updateObj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_BRACES_updateObj_I, ELEMS_TUP)

/// @brief Expand the variables for updateObj
#define EXPAND_VARIABLES_updateObj_I(z, n, ELEMS_TUP) BOOST_PP_COMMA()BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))BOOST_PP_COMMA()obj->BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)
#define EXPAND_VARIABLES_updateObj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_VARIABLES_updateObj_I, ELEMS_TUP)

/// @brief getObj Helper Macros
/// @brief getObj will get a autoptr to 

/// @brief Assorted Helper Macros
/// @brief Create n number of string
#define Repeat_N_String_With_Precomma_I(z, n, text)  BOOST_PP_IDENTITY(",")() BOOST_STRINGIZE(text)
#define Repeat_N_String_With_Precomma(n, text) BOOST_PP_REPEAT(n, Repeat_N_String_With_Precomma_I, text)

/// @brief Macro to output comments
#define BLIB_MACRO_COMMENTS_ENABLED 0
#define BLIB_MACRO_COMMENTS_IF(a) BOOST_PP_EXPR_IF(BLIB_MACRO_COMMENTS_ENABLED, a)

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

/// SPECIALIZE_BUN_HELPER Start
#define SPECIALIZE_BUN_HELPER(CLASS_ELEMS_TUP) namespace soci{\
"@brief --Specialization for SOCI ORM Start---";\
BLIB_MACRO_COMMENTS_IF("@brief --Specialization for SOCI ORM Start---");\
template<>\
struct type_conversion<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
typedef values base_type;\
using ClassType = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
inline static void from_base(values const& v, indicator ind, ClassType& c){\
BLIB_MACRO_COMMENTS_IF("@brief from_base gets the values from db");\
EXPAND_MEMBER_ASSIGNENTS_from_base(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ));\
}\
inline static void to_base(const ClassType& c, values& v, indicator& ind){\
BLIB_MACRO_COMMENTS_IF("@brief to_base puts the values to db");\
EXPAND_MEMBER_ASSIGNENTS_to_base(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ));\
}\
};\
}\
namespace blib{namespace bun{namespace __private{\
BLIB_MACRO_COMMENTS_IF("@brief --Specialization for PRefHelper Start---");\
template<>\
struct PRefHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
using T = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
\
inline static void createSchema(){\
BLIB_MACRO_COMMENTS_IF("@brief createSchema for creating the schema of an object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const query = "CREATE TABLE IF NOT EXISTS '{}' (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER NOT NULL" \
EXPAND_CLASS_MEMBERS_createSchema(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) \
 ")";\
static std::string const sql = fmt::format(query, class_name, EXPAND_CLASS_TYPE_MEMBERS_createSchema(CLASS_ELEMS_TUP));\
QUERY_LOG(sql);\
blib::bun::__private::DbBackend<>.i().session() << sql;\
}\
\
inline static void deleteSchema(){\
BLIB_MACRO_COMMENTS_IF("@brief deleteSchema for deleting the schema of an object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const sql = fmt::format("DROP TABLE '{}'", class_name);\
QUERY_LOG(sql);\
blib::bun::__private::DbBackend<>.i().session() << sql;\
}\
\
inline static SimpleOID persistObj( T* obj ){\
BLIB_MACRO_COMMENTS_IF("@brief persistObj for persisting the object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
SimpleOID oid;\
oid.populateLow();\
static std::string const query = "INSERT INTO '{}' (oid_low" \
EXPAND_FIELDS_persistObj(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))\
") VALUES ({}" \
Repeat_N_String_With_Precomma(BOOST_PP_TUPLE_SIZE(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )), {})\
;\
std::string const sql = fmt::format(query, class_name, oid.low,\
EXPAND_VARIABLES_persistObj(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))\
);\
QUERY_LOG(sql);\
blib::bun::__private::DbBackend<>.i().session() << sql, use(*obj);\
return oid;\
}\
inline static void updateObj( T* obj, SimpleOID const& oid ){\
BLIB_MACRO_COMMENTS_IF("@brief updateObj for updating a persisted object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const query = "UPDATE {} SET " \
EXPAND_BRACES_updateObj(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) \
" WHERE oid_low={} AND oid_high={}";\
std::string const sql = fmt::format(query, class_name \
EXPAND_VARIABLES_updateObj(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) \
,oid.low, oid.high);\
QUERY_LOG(sql);\
blib::bun::__private::DbBackend<>.i().session() << sql, use(*obj);\
}\
inline static void deleteObj( SimpleOID const& oid ){\
BLIB_MACRO_COMMENTS_IF("@brief deleteObj for deleting a persisted object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const query = "DELETE FROM {} WHERE oid_high={} AND oid_low={}";\
std::string const sql = fmt::format(query, class_name, oid.high, oid.low);\
QUERY_LOG(sql);\
blib::bun::__private::DbBackend<>.i().session() << sql;\
}\
inline static std::unique_ptr<T> getObj( SimpleOID const& oid ){\
BLIB_MACRO_COMMENTS_IF("@brief getObj for getting a persisted object with the oid");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
std::unique_ptr<T> obj = std::make_unique<T>();\
static std::string const query = "SELECT * FROM {} WHERE oid_high={} AND oid_low={}";\
std::string const sql = fmt::format(query, class_name, oid.high, oid.low);\
QUERY_LOG(sql);\
blib::bun::__private::DbBackend<>.i().session() << sql, into(*obj);\
return std::move(obj);\
}\
};\
BLIB_MACRO_COMMENTS_IF("@brief ---Specialization for PRefHelper End---");\
BLIB_MACRO_COMMENTS_IF("@brief ---===---");\
BLIB_MACRO_COMMENTS_IF("@brief --Specialization for QueryHelper Start---");\
template<>\
struct QueryHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
using T = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
inline static std::vector<SimpleOID> getAllOids(){\
BLIB_MACRO_COMMENTS_IF("@brief getAllOids for getting all oids of the persistant objects for this class.");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
return std::vector<SimpleOID>();\
}\
inline static std::vector<PRef<T>> getAllObjects(){\
BLIB_MACRO_COMMENTS_IF("@brief getAllObjects for getting all objects of the persistant objects for this class.");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
return std::vector<PRef<T>>();\
}\
};\
BLIB_MACRO_COMMENTS_IF("@brief ---Specialization for QueryHelper End---");\
}\
}\
}\

/// SPECIALIZE_BUN_HELPER End


///////////////////////////////////////////////////////////////////////////////
/// @basic Basic Persistance End
///////////////////////////////////////////////////////////////////////////////
