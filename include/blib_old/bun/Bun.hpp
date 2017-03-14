#pragma once

/////////////////////////////////////////////////
/// @file Bun.hpp
/// @author BrainlessLabs
/// @version 0.2
/// @brief The include file for Bun ORM. This file includes all the source
///        to make the class/struct persist in sqlite
/////////////////////////////////////////////////
#include "blib/utils/Singleton.hpp"
#include "blib/utils/MD5.hpp"
#include "blib/utils/TypeUtils.hpp"
#include "blib/bun/CppTypeToSQLString.hpp"
#include "blib/bun/Query.hpp"
#include "blib/bun/PRef.hpp"
#include "blib/bun/DbBackend.hpp"
#include "blib/bun/DbLogger.hpp"
#include <boost/preprocessor.hpp>
#include <blib/fmt/format.hpp>
#include <blib/rapidjson/rapidjson.h>
#include <memory>
#include <tuple>
#include <array>
#include <ratio>
#include <stack>
#include <vector>
#include <type_traits>

namespace blib {
  namespace bun {

    /////////////////////////////////////////////////
    /// @class BunHelper
    /// @brief Helper class for the persistent framework.
    ///        This class is specialized to persist objects.
    /////////////////////////////////////////////////
    template<typename T>
    struct BunHelper {
      inline static void createTable();
      inline static SimpleOID persistObj( T* );
      inline static void updateObj( T*, SimpleOID const& );
      inline static void deleteObj( SimpleOID const& );
      inline static std::unique_ptr<T> getObj( SimpleOID const& );
      inline static std::string md5( T*, SimpleOID const& );
      inline static std::string objToString( T*, SimpleOID const& );
      inline static std::string objToJson( T*, SimpleOID const& );
    };

    template<typename T>
    inline bool createSchema();

    template<typename T>
    inline bool dropSchema();

    template<typename T>
    inline std::vector<SimpleOID> getAllOids();

    template<typename T>
    inline std::vector<PRef<T>> getAllObjects();

    /// Query namespace
    namespace query {
      namespace _details {
        template<typename T>
        inline std::vector<blib::bun::PRef<T>> getAllObjWithQuery( std::string const& in_query );

        template<typename T>
        inline std::string const& className();
      }
    }
  }
}


///----------------------------------------------------------------------------
/// Basic Persistance Start

/// CREATE_SCHEMA_TYPES
#define GET_DB_TYPE(CLASS_NAME, ELEMENT) cppTypeToDbTypeString<decltype(CLASS_NAME:: ## ELEMENT)>()
#define CREATE_SCHEMA_TYPES_I(z, n, CLASS_TUP) BOOST_PP_COMMA()BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n, 1), CLASS_TUP))\
BOOST_PP_COMMA() GET_DB_TYPE(BOOST_PP_TUPLE_ELEM(0, CLASS_TUP), BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n, 1), CLASS_TUP))

#define CREATE_SCHEMA_TYPES(CLASS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_TUP), 1), CREATE_SCHEMA_TYPES_I, CLASS_TUP)

/// REPEAT_WITH_COMMA
#define REPEAT_WITH_COMMA_I(z, n, TEXT) BOOST_PP_IF(n, ",", "") TEXT
#define REPEAT_WITH_COMMA(CLASS_TUP, TEXT) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_TUP), REPEAT_WITH_COMMA_I, BOOST_PP_STRINGIZE(TEXT))

/// BunHelper
#define OBJ_TO_STRING_I(z, n, CLASS_ELEMS_TUP) ,_details::_E(in_obj->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define OBJ_TO_STRING(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), OBJ_TO_STRING_I, CLASS_ELEMS_TUP)

#define UPDATE_STATEMENT_QUERY_I(z, n, CLASS_ELEMS_TUP) ,_details::_E(in_obj->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define UPDATE_STATEMENT_QUERY(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), UPDATE_STATEMENT_QUERY_I, CLASS_ELEMS_TUP)

#define UPDATE_STATEMENT_QUERY_STR_CAT(ELEM) BOOST_PP_STRINGIZE(ELEM) "= {}"
#define UPDATE_STATEMENT_QUERY_STR_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, ",", "") UPDATE_STATEMENT_QUERY_STR_CAT(BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define UPDATE_STATEMENT_QUERY_STR(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), UPDATE_STATEMENT_QUERY_STR_I, CLASS_ELEMS_TUP)

#define GENERATE_GETOBJ_MEMBERS_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_COMMA_IF(n)ret->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP)
#define GENERATE_GETOBJ_MEMBERS(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), GENERATE_GETOBJ_MEMBERS_I, CLASS_ELEMS_TUP)

#define GENERATE_TABLE_COLUMN_NAMES_PTR_I(z, n, CLASS_ELEMS_TUP) ,_details::_E(in_obj->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define GENERATE_TABLE_COLUMN_NAMES_PTR(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), GENERATE_TABLE_COLUMN_NAMES_PTR_I, CLASS_ELEMS_TUP)

#define GENERATE_TABLE_COLUMN_NAMES_I(z, n, CLASS_ELEMS_TUP) ,BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define GENERATE_TABLE_COLUMN_NAMES(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), GENERATE_TABLE_COLUMN_NAMES_I, CLASS_ELEMS_TUP)

#define GENERATE_DB_HELPER(CLASS_ELEMS_TUP) namespace blib{namespace bun{\
template<>\
struct BunHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> {\
using T = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
inline static void createTable(){\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)); \
static const std::string sql = fmt::format( "CREATE TABLE IF NOT EXISTS '{}'(object_id INTEGER NOT NULL," REPEAT_WITH_COMMA( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {} {} ) ")", \
  class_name CREATE_SCHEMA_TYPES( CLASS_ELEMS_TUP ));\
l().info() << sql;\
blib::bun::Db::i().db() << sql;\
}\
\
inline static SimpleOID persistObj( T* in_obj){\
SimpleOID oid;\
oid.populateLow();\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
const std::string sql = fmt::format(\
"INSERT INTO '{}' (object_id," REPEAT_WITH_COMMA( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {} ) ") VALUES({}," REPEAT_WITH_COMMA(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {}) ")", class_name \
GENERATE_TABLE_COLUMN_NAMES(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )), oid.low \
GENERATE_TABLE_COLUMN_NAMES_PTR(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))\
);\
l().info() << sql;\
Db::i().db() << sql;\
oid.high = Db::i().db().last_insert_rowid();\
return oid;\
}\
\
inline static void updateObj( T* in_obj, SimpleOID const& in_oid){\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
const std::string sql = fmt::format(\
"UPDATE '{}' SET " UPDATE_STATEMENT_QUERY_STR(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) " WHERE object_id = {} AND rowid = {}", class_name \
UPDATE_STATEMENT_QUERY(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )), in_oid.low, in_oid.high);\
l().info() << sql;\
Db::i().db() << sql;\
}\
\
inline static std::unique_ptr<T> getObj( SimpleOID const& in_oid ){\
const std::string sql = fmt::format(\
  "SELECT " REPEAT_WITH_COMMA( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {} ) " FROM '{}' WHERE object_id = {} AND rowid = {}"\
  GENERATE_TABLE_COLUMN_NAMES( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ) ), BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )), in_oid.low, in_oid.high \
);\
l().info() << sql;\
std::unique_ptr<T> ret( new T );\
Db::i().db() << sql >> std::tie(GENERATE_GETOBJ_MEMBERS(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )));\
return ret;\
}\
\
inline static void deleteObj(SimpleOID const& in_oid){ \
static const std::string class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
const std::string sql = fmt::format("DELETE FROM '{}' WHERE object_id = {} AND rowid = {}", class_name, in_oid.low, in_oid.high);\
}\
\
inline static std::string objToString( T* in_obj, SimpleOID const& in_oid ){\
const std::string ret = fmt::format(\
"{},{},{}," REPEAT_WITH_COMMA(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {}), BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )), in_oid.high, in_oid.low \
OBJ_TO_STRING(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )));\
l().info() << ret;\
return ret;\
}\
\
inline static std::string md5( T* in_obj, SimpleOID const& in_oid ) {\
const std::string str = objToString( in_obj, in_oid );\
const std::string ret = blib::md5( str );\
return ret;\
}\
inline static std::string objToJson(T* in_obj, SimpleOID const& in_oid){\
std::string ret = "";\
return ret;\
}\
\
};\
}}


/// REGISTER_SCHEMA
#define REGISTER_SCHEMA(CLASS_ELEMS_TUP) namespace blib{namespace bun{\
template<>\
inline bool createSchema<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>(){ \
BunHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>::createTable();\
return true;\
}\
\
template<>\
inline bool dropSchema<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>(){ \
static const std::string class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static const std::string sql = fmt::format("DROP TABLE '{}'", class_name);\
l().info() << sql;\
Db::i().db() << sql;\
return true;\
}\
}}

/// Get all of the items (just the Oids)
#define REGISTER_GETALLOIDS(CLASS_ELEMS_TUP) namespace blib{namespace bun{\
template<>\
inline std::vector<SimpleOID> getAllOids<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>() {\
static const std::string sql = fmt::format("SELECT rowid, object_id FROM '{}'", BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)));\
std::vector<SimpleOID> ret;\
l().info() << sql;\
Db::i().db() << sql >> [&]( const std::int64_t in_high, const std::int64_t in_low ) {\
ret.push_back(SimpleOID{static_cast<decltype(SimpleOID::high)>(in_high), static_cast<decltype(SimpleOID::low)>(in_low)});};\
return ret;\
}\
}\
}

/// Get all of the items (just the Objects)
#define EXPAND_ASSIGN_VAR_GETALLOBJ_I(z, n, CLASS_ELEMS_TUP) ;r->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP)=BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP)
#define EXPAND_ASSIGN_VAR_GETALLOBJ(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), EXPAND_ASSIGN_VAR_GETALLOBJ_I, CLASS_ELEMS_TUP)

#define GET_OBJ_NAME_GETALLOBJ(CLASS_ELEMS_TUP, n) BOOST_PP_TUPLE_ELEM(BOOST_PP_INC(n), CLASS_ELEMS_TUP)
#define GET_OBJ_TYPE_GETALLOBJ(CLASS_ELEMS_TUP, n) BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_INC(n), CLASS_ELEMS_TUP)
#define EXPAND_VARS_FOR_FUN_GETALLOBJ_I(z, n, CLASS_ELEMS_TUP) ,const blib::bun::_details::BoxingType<decltype(GET_OBJ_TYPE_GETALLOBJ(CLASS_ELEMS_TUP, n))>::type GET_OBJ_NAME_GETALLOBJ(CLASS_ELEMS_TUP, n)
#define EXPAND_VARS_FOR_FUN_GETALLOBJ(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_DEC(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP)), EXPAND_VARS_FOR_FUN_GETALLOBJ_I, CLASS_ELEMS_TUP)

#define EXPAND_OBJ_FOR_GETALLOBJ_I(z, n, CLASS_ELEMS_TUP) "," BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define EXPAND_OBJ_FOR_GETALLOBJ(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), EXPAND_OBJ_FOR_GETALLOBJ_I, CLASS_ELEMS_TUP)

#define REGISTER_GETALLOBJECTS(CLASS_ELEMS_TUP) namespace blib{namespace bun{\
template<>\
inline std::vector<PRef<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>> getAllObjects<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>(){\
const static std::string class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
const static std::string sql = fmt::format(\
"SELECT rowid, object_id " EXPAND_OBJ_FOR_GETALLOBJ(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) " FROM '{}'", class_name);\
l().info() << sql;\
std::vector<PRef<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>> ret;\
Db::i().db() << sql >> [&](const std::int64_t in_high, const std::int64_t in_low EXPAND_VARS_FOR_FUN_GETALLOBJ(CLASS_ELEMS_TUP)){\
const SimpleOID oid(in_high, in_low);\
auto r = std::make_unique<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>();\
EXPAND_ASSIGN_VAR_GETALLOBJ(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ));\
ret.emplace_back(oid, r.release());\
};\
return ret;\
}\
}\
}

// Tells that this class can be persisted.
#define REGISTER_CAN_PERSIST(CLASS_NAME) namespace blib{namespace bun{ template<> struct IsPersistant<CLASS_NAME> : std::true_type {}; } }

/// Basic Persistance End
///----------------------------------------------------------------------------

///----------------------------------------------------------------------------
/// Bun Query Preprocessor Start
#define DEFINE_CLASS_STATIC_VARS_QUERY_I(z, n, CLASS_ELEMS_TUP) boost::proto::terminal<blib::bun::query::_details::QueryVariablePlaceholderIndex<n>>::type const F<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n, 1), CLASS_ELEMS_TUP);
#define DEFINE_CLASS_STATIC_VARS_QUERY(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), DEFINE_CLASS_STATIC_VARS_QUERY_I, CLASS_ELEMS_TUP)

#define GENERATE_CLASS_STATIC_VARS_QUERY_I(z, n, ELEMS_TUP) static boost::proto::terminal<blib::bun::query::_details::QueryVariablePlaceholderIndex<n>>::type const BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP);
#define GENERATE_CLASS_STATIC_VARS_QUERY(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), GENERATE_CLASS_STATIC_VARS_QUERY_I, ELEMS_TUP)

#define GENERATE_TABLE_COLUMN_NAMES_QUERY_I(z, n, ELEMS_TUP) BOOST_PP_COMMA_IF(n)BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define GENERATE_TABLE_COLUMN_NAMES_QUERY(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), GENERATE_TABLE_COLUMN_NAMES_QUERY_I, ELEMS_TUP)

#define REGISTER_QUERY_HANDLERS(CLASS_ELEMS_TUP) namespace blib { namespace bun { namespace query { namespace _details {\
template<>\
inline std::vector<blib::bun::PRef<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>> \
getAllObjWithQuery<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>( std::string const& in_query ){\
const static std::string class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
const std::string sql = fmt::format(\
"SELECT rowid, object_id " EXPAND_OBJ_FOR_GETALLOBJ(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) " FROM '{}' WHERE {}", class_name, in_query);\
l().info() << sql;\
std::vector<blib::bun::PRef<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>> ret;\
Db::i().db() << sql >> [&](const std::int64_t in_high, const std::int64_t in_low EXPAND_VARS_FOR_FUN_GETALLOBJ(CLASS_ELEMS_TUP)){\
const SimpleOID oid(in_high, in_low);\
auto r = std::make_unique<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>();\
EXPAND_ASSIGN_VAR_GETALLOBJ(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ));\
ret.emplace_back(oid, r.release());\
};\
return std::move(ret);\
}\
\
template<> inline std::string const& className<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>(){\
static const std::string class_name = BOOST_PP_STRINGIZE( BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP ) ); \
return class_name;\
}\
\
template<> inline std::string const& mapping<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>( const std::uint32_t in_index ){\
static const std::vector<std::string> class_fields = { GENERATE_TABLE_COLUMN_NAMES_QUERY(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) };\
return class_fields.at( in_index );\
}\
\
template<>\
struct TypesUsed<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )> { \
    typedef void Type;\
};\
\
}\
\
\
namespace {\
template<>\
struct F<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> {\
GENERATE_CLASS_STATIC_VARS_QUERY(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))\
};\
\
DEFINE_CLASS_STATIC_VARS_QUERY(CLASS_ELEMS_TUP)\
\
}\
}\
}\
}

/// Bun Query Preprocessor Query End
///----------------------------------------------------------------------------

// Starting doing of all the registrations
#define GENERATE_BINDING(CLASS_ELEMS_TUP) \
REGISTER_CAN_PERSIST(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)) \
GENERATE_DB_HELPER(CLASS_ELEMS_TUP) \
REGISTER_SCHEMA(CLASS_ELEMS_TUP) \
REGISTER_GETALLOIDS(CLASS_ELEMS_TUP) \
REGISTER_GETALLOBJECTS(CLASS_ELEMS_TUP) \
REGISTER_QUERY_HANDLERS(CLASS_ELEMS_TUP)