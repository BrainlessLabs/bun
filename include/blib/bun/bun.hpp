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
#include "blib/bun/GlobalFunc.hpp"
#include "blib/utils/JSONUtils.hpp"

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
#define EXPAND_MEMBER_ASSIGNENTS_from_base_I(z, n, ELEMS_TUP) c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP) = v.get<blib::bun::__private::ConvertCPPTypeToSOCISupportType<decltype(c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)));
#define EXPAND_MEMBER_ASSIGNENTS_from_base(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_MEMBER_ASSIGNENTS_from_base_I, ELEMS_TUP)

/// @details to_base
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
#define EXPAND_MEMBER_ASSIGNENTS_to_base_I(z, n, ELEMS_TUP) v.set(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)), blib::bun::__private::convertToSOCISupportedType(c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)));
#define EXPAND_MEMBER_ASSIGNENTS_to_base(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_MEMBER_ASSIGNENTS_to_base_I, ELEMS_TUP)

/// @brief createSchema Helper Macros
/// @details We need to pass only the data members as a tuple to this macro
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
/// @brief Expands the class members for CREATE TABLE
#define EXPAND_CLASS_MEMBERS_createSchema_I(z, n, ELEMS_TUP) ", " BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)) " {}"
#define EXPAND_CLASS_MEMBERS_createSchema(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_CLASS_MEMBERS_createSchema_I, ELEMS_TUP)

/// @brief Expands the class members for CREATE TABLE to get the type info
#define EXPAND_CLASS_TYPE_MEMBERS_createSchema_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_COMMA_IF(n) blib::bun::cppTypeToDbTypeString<ConvertCPPTypeToSOCISupportType<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n,1), CLASS_ELEMS_TUP))>::type>()
#define EXPAND_CLASS_TYPE_MEMBERS_createSchema(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), EXPAND_CLASS_TYPE_MEMBERS_createSchema_I, CLASS_ELEMS_TUP)

/// @brief persistObj Helper Macros
/// @brief Changes related to INSERT INTO
/// @brief EXPAND_CLASS_TYPE_MEMBERS_persistObj
#define EXPAND_FIELDS_persistObj_I(z, n, ELEMS_TUP) BOOST_PP_IDENTITY(",")() BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_FIELDS_persistObj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_FIELDS_persistObj_I, ELEMS_TUP)

/// @brief Expand the variables for persistObj
#define EXPAND_VARIABLES_persistObj_I(z, n, ELEMS_TUP) BOOST_PP_COMMA_IF(n) ":" BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_VARIABLES_persistObj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_VARIABLES_persistObj_I, ELEMS_TUP)

/// @brief updateObj Helper Macros
/// @brief Changes related to UPDATE table SET
/// @brief Expand the braces for updateObj
#define EXPAND_BRACES_updateObj_I(z, n, ELEMS_TUP) BOOST_PP_EXPR_IF(n, ",")BOOST_STRINGIZE({} = {})
#define EXPAND_BRACES_updateObj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_BRACES_updateObj_I, ELEMS_TUP)

/// @brief Expand the variables for updateObj
#define EXPAND_VARIABLES_updateObj_I(z, n, ELEMS_TUP) BOOST_PP_COMMA()BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))BOOST_PP_COMMA() ":" BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_VARIABLES_updateObj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_VARIABLES_updateObj_I, ELEMS_TUP)

/// @brief objToJson Helper Macros
/// @brief objToJson Generate the static string format
#define EXPAND_VARIABLES_objToJson_static_I(z, n, ELEMS_TUP) "," BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)) ":{}"
#define EXPAND_VARIABLES_objToJson_static(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_VARIABLES_objToJson_static_I, ELEMS_TUP)

/// @brief objToJson Expand the object
#define EXPAND_VARIABLES_objToJson_expand_obj_I(z, n, ELEMS_TUP) BOOST_PP_COMMA_IF(n) blib::bun::tojson_string(obj->BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_VARIABLES_objToJson_expand_obj(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_VARIABLES_objToJson_expand_obj_I, ELEMS_TUP)

/// @brief QueryHelper Macros
/// @brief Expand the member for the query in getAllObjWithQuery
/// @brief EXPAND_CLASS_TYPE_MEMBERS_getAllObjWithQuery
#define EXPAND_CLASS_TYPE_MEMBERS_getAllObjWithQuery_I(z, n, ELEMS_TUP) "," BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_CLASS_TYPE_MEMBERS_getAllObjWithQuery(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_CLASS_TYPE_MEMBERS_getAllObjWithQuery_I, ELEMS_TUP)

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
try{\
blib::bun::__private::DbBackend<>::i().session() << sql;\
}catch(std::exception const & e){\
l().error("createSchema: {} ", e.what());\
}\
}\
\
inline static void deleteSchema(){\
BLIB_MACRO_COMMENTS_IF("@brief deleteSchema for deleting the schema of an object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const sql = fmt::format("DROP TABLE '{}'", class_name);\
QUERY_LOG(sql);\
try{\
blib::bun::__private::DbBackend<>::i().session() << sql;\
}catch(std::exception const & e){\
l().error("deleteSchema: {} ", e.what());\
}\
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
")";\
std::string const sql = fmt::format(query, class_name, oid.low,\
EXPAND_VARIABLES_persistObj(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))\
);\
QUERY_LOG(sql);\
try{\
blib::bun::__private::DbBackend<>::i().session() << sql, use(*obj);\
}catch(std::exception const & e){\
l().error("persistObj: {} ", e.what());\
}\
return oid;\
}\
inline static void updateObj( T* obj, SimpleOID const& oid ){\
BLIB_MACRO_COMMENTS_IF("@brief updateObj for updating a persisted object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const query = "UPDATE '{}' SET " \
EXPAND_BRACES_updateObj(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) \
" WHERE oid_low={} AND oid_high={}";\
std::string const sql = fmt::format(query, class_name \
EXPAND_VARIABLES_updateObj(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) \
,oid.low, oid.high);\
QUERY_LOG(sql);\
try{\
blib::bun::__private::DbBackend<>::i().session() << sql, use(*obj);\
}catch(std::exception const & e){\
l().error("updateObj: {} ", e.what());\
}\
}\
inline static void deleteObj( SimpleOID const& oid ){\
BLIB_MACRO_COMMENTS_IF("@brief deleteObj for deleting a persisted object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const query = "DELETE FROM '{}' WHERE oid_high={} AND oid_low={}";\
std::string const sql = fmt::format(query, class_name, oid.high, oid.low);\
QUERY_LOG(sql);\
try{\
blib::bun::__private::DbBackend<>::i().session() << sql;\
}catch(std::exception const & e){\
l().error("deleteObj: {} ", e.what());\
}\
}\
inline static std::unique_ptr<T> getObj( SimpleOID const& oid ){\
BLIB_MACRO_COMMENTS_IF("@brief getObj for getting a persisted object with the oid");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
std::unique_ptr<T> obj = std::make_unique<T>();\
static std::string const query = "SELECT * FROM {} WHERE oid_high={} AND oid_low={}";\
std::string const sql = fmt::format(query, class_name, oid.high, oid.low);\
QUERY_LOG(sql);\
try{\
blib::bun::__private::DbBackend<>::i().session() << sql, into(*obj);\
}catch(std::exception const & e){\
l().error("getObj: {} ", e.what());\
}\
return std::move(obj);\
}\
inline static std::string objToJson(T *obj, SimpleOID const &oid){\
BLIB_MACRO_COMMENTS_IF("@brief objToJson getting the string representation of object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const obj_json_str = std::string("class:{},oid_high: {},oid_low:{}") + std::string(EXPAND_VARIABLES_objToJson_static(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )));\
std::string obj_json;\
try{\
obj_json = fmt::format(obj_json_str, class_name, oid.high, oid.low, EXPAND_VARIABLES_objToJson_expand_obj(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )));\
}catch(std::exception const & e){\
l().error("objToJson: {} ", e.what());\
}\
obj_json = "{" + obj_json + "}";\
return std::move(obj_json);\
}\
inline static std::string objToString(T *obj, SimpleOID const & oid){\
BLIB_MACRO_COMMENTS_IF("@brief objToString getting the JSON representation of object");\
return std::move(objToJson(obj, oid));\
}\
inline static std::string md5(T *obj, SimpleOID const & oid){\
BLIB_MACRO_COMMENTS_IF("@brief md5 getting the md5 string of object");\
const std::string md5 = blib::md5(objToString(obj, oid));\
return std::move(md5);\
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
const std::vector<PRef<T>> ret = getAllObjWithQuery("");\
return std::move(ret);\
}\
inline static std::vector<PRef<T>> getAllObjWithQuery(std::string const &in_query){\
BLIB_MACRO_COMMENTS_IF("@brief getAllObjWithQuery for getting all objects of the persistant objects for this class with the provided query.");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
const std::string query = "SELECT oid_high, oid_low" \
EXPAND_CLASS_TYPE_MEMBERS_getAllObjWithQuery(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) \
"FROM {} {}";\
const std::string where_clasue = in_query.empty() ? "" : "WHERE " + in_query;\
std::vector<PRef<T>> ret;\
try{\
const std::string sql = fmt::format(query, class_name, where_clasue);\
QUERY_LOG(sql);\
row r;\
blib::bun::__private::DbBackend<>::i().session() << sql, soci::into(r);\
const blib::bun::SimpleOID oid();\
}\
catch (std::exception const & e) {\
l().error("objToJson: {} ", e.what());\
}\
return std::move(ret);\
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
