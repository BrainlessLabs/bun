
#pragma once

#include <hdr/sqlite_modern_cpp.hpp>
#include <boost/preprocessor.hpp>
#include <fmt/format.hpp>
#include <eggs/variant.hpp>
#include <spdlog/spdlog.h>
#include "blib/utils/Singleton.hpp"
#include "blib/utils/MD5.hpp"
#include <memory>
#include <tuple>
#include <bitset>
#include <array>
#include <chrono>
#include <ratio>
#include <stack>
#include <type_traits>

namespace blib {
  namespace bun {
    inline std::shared_ptr<spdlog::logger>& l() {
      static const size_t q_size = 1048576; //queue size must be power of 2
      spdlog::set_async_mode( q_size );
      static auto ret = spdlog::daily_logger_st( "async_file_logger", "async_log.txt" );
      return ret;
    }

    struct Db : public ::blib::Singleton<Db> {
      using DbConnectionType = sqlite::database;
      std::unique_ptr<DbConnectionType> _db;
      bool _ok;
      std::string _fileName;

      bool ok() const {
        return _ok;
      }

      DbConnectionType& db() {
        return *_db;
      }

      bool connect( std::string const& aDbName ) {
        bool ret = true;
        try {
          _db.reset( new DbConnectionType( aDbName ) );
          _ok = true;
          _fileName = aDbName;
        }
        catch (...) {
          ret = false;
          _ok = false;
        }
        return ret;
      }
    };

    struct SimpleOID {
      typedef SimpleOID SelfType;
      typedef std::array<std::uint8_t, 16> OidByteArrayType;
      std::uint64_t high;
      std::uint64_t low;

      SimpleOID() = default;
      SimpleOID( const std::uint64_t in_high ) :
        high( in_high ),
        low( 0 ) {
        populateLow();
      }

      SimpleOID( const std::uint64_t in_high, const std::uint64_t in_low ) :
        high( in_high ),
        low( in_low ) {
      }

      void populateLow() {
        const auto t = std::chrono::high_resolution_clock::now();
        low = t.time_since_epoch().count();
      }

      SelfType& operator=( SelfType const& in_other ) {
        high = in_other.high;
        low = in_other.low;
      }

      bool operator==( SelfType const& in_other ) const {
        return high == in_other.high && low == in_other.low;
      }

      bool operator!=( SelfType const& in_other ) const {
        return !operator==( in_other );
      }

      OidByteArrayType toByteArray() const {
        union Convert {
          std::uint64_t var;
          std::uint8_t convert[8];
        };
        OidByteArrayType ret{ 0 };
        const Convert hc = { high };
        for (int i = 0; i < 8; ++i) {
          ret[i] = hc.convert[i];
        }

        const Convert lc = { low };
        for (int i = 0, j = 8; i < 8; ++i, ++j) {
          ret[j] = lc.convert[i];
        }

        return ret;
      }
    };

    enum class DbTypes : std::uint32_t {
      kUnknown = 0,
      kInteger,
      kReal,
      kNumeric,
      kText
    };

    template<typename T>
    struct CppTypeToDbType {
      static const DbTypes ret = DbTypes::kUnknown;
    };

    template<>
    struct CppTypeToDbType<int> {
      static const DbTypes ret = DbTypes::kInteger;
    };

    template<>
    struct CppTypeToDbType<unsigned int> {
      static const DbTypes ret = DbTypes::kInteger;
    };

    template<>
    struct CppTypeToDbType<char> {
      static const DbTypes ret = DbTypes::kInteger;
    };

    template<>
    struct CppTypeToDbType<unsigned char> {
      static const DbTypes ret = DbTypes::kInteger;
    };

    template<>
    struct CppTypeToDbType<float> {
      static const DbTypes ret = DbTypes::kReal;
    };

    template<>
    struct CppTypeToDbType<double> {
      static const DbTypes ret = DbTypes::kReal;
    };

    template<>
    struct CppTypeToDbType<std::string> {
      static const DbTypes ret = DbTypes::kText;
    };

    template<>
    struct CppTypeToDbType<std::wstring> {
      static const DbTypes ret = DbTypes::kText;
    };

    // Convert cpp type to sqlite type
    template<DbTypes N = DbTypes::kUnknown>
    inline std::string const& cppTypeEnumToDbTypeString() {
      const static std::string ret = "UNKNOWN";
      return ret;
    }

    template<>
    inline std::string const& cppTypeEnumToDbTypeString<DbTypes::kInteger>() {
      const static std::string ret = "INTEGER";
      return ret;
    }

    template<>
    inline std::string const& cppTypeEnumToDbTypeString<DbTypes::kNumeric>() {
      const static std::string ret = "NUMERIC";
      return ret;
    }

    template<>
    inline std::string const& cppTypeEnumToDbTypeString<DbTypes::kReal>() {
      const static std::string ret = "REAL";
      return ret;
    }

    template<>
    inline std::string const& cppTypeEnumToDbTypeString<DbTypes::kText>() {
      const static std::string ret = "TEXT";
      return ret;
    }

    template<typename T>
    inline std::string const& cppTypeToDbTypeString() {
      return cppTypeEnumToDbTypeString<CppTypeToDbType<T>::ret>();
    }

    template<typename T>
    inline bool canPersist() {
      return false;
    }

    template<>
    inline bool canPersist<int>() {
      return true;
    }

    template<>
    inline bool canPersist<unsigned int>() {
      return true;
    }

    template<>
    inline bool canPersist<char>() {
      return true;
    }

    template<>
    inline bool canPersist<unsigned char>() {
      return true;
    }

    template<>
    inline bool canPersist<float>() {
      return true;
    }

    template<>
    inline bool canPersist<double>() {
      return true;
    }

    template<>
    inline bool canPersist<std::string>() {
      return true;
    }

    template<typename T>
    struct BunHelper {
      inline static void createTable();
      inline static SimpleOID persistObj( T* );
      inline static void updateObj( T*, SimpleOID const& );
      inline static std::unique_ptr<T> getObj( SimpleOID const& );
      inline static std::string md5( T*, SimpleOID const& );
      inline static std::string objToString( T*, SimpleOID const& );
      inline static std::string objToJson( T*, SimpleOID const& );
    };

    template<typename T>
    inline bool createSchema();

    // Query

    // Persistance reference holder
    template<typename T>
    class PRef {
    private:
      enum class FlagsE : std::uint8_t {
        kDirty = 0
      };

      typedef SimpleOID OidType;
      std::unique_ptr<T> _obj;
      std::bitset<4> _flags;
      std::string _md5;

    public:
      typedef T ObjType;
      typedef PRef<T> SelfType;
      OidType oid;

    public:
      PRef() = default;
      PRef( ObjType* in_obj ) {
        _obj.reset( in_obj );
      }

      PRef( OidType const& );

      void reset( ObjType* in_obj ) {
        _obj.reset( in_obj );
        _flags.reset();
        _md5 = "";
      }

      ObjType* release() {
        oid.high = 0;
        oid.low = 0;
        _flags.reset();
        _md5 = "";
        return _obj.release();
      }

      ~PRef() = default;

      bool dirty() {
        const auto md5 = BunHelper<ObjType>::md5( _obj.get(), oid );
        if (md5 != _md5) {
          _flags[static_cast<std::uint8_t>(FlagsE::kDirty)] = 1;
        }

        return _flags[static_cast<std::uint8_t>(FlagsE::kDirty)] ? true : false;
      }

      OidType persist() {
        if (_md5.empty()) {
          oid = BunHelper<ObjType>::persistObj( _obj.get() );
        }
        else {
          BunHelper<ObjType>::updateObj( _obj.get() );
        }
        _md5 = BunHelper<ObjType>::md5( _obj.get(), oid );
        _flags.reset();
        return oid;
      }

      void load( OidType const& in_oid ) {
        oid = in_oid;
        _obj = BunHelper<ObjType>::getObj( oid );
        _md5 = BunHelper<ObjType>::md5( _obj.get(), oid );
        _flags.reset();
      }

      PRef& operator=( ObjType* in_obj ) {
        reset( in_obj );
        return *this;
      }

      PRef& operator=( PRef& in_other ) {
        oid = in_other.oid;
        _md5 = in_other._md5;
        _flags = in_other._flags;
        _obj = in_other._obj;
        return *this;
      }

      bool operator==( PRef const& in_other ) {
        return oid == in_other.oid;
      }

      bool operator!=( PRef const& in_other ) {
        return oid != in_other.oid;
      }

      std::string toJson() const {
        return BunHelper<T>::objToJson( _obj.get(), oid );
      }
    };
    template<typename T>
    class Query;

    namespace _private {

      enum class OperatorE : std::uint32_t {
        kGreaterThan = 0, // >
        kGreaterThanEqualTo, // >=
        kLessThan, // <
        kLessThanEqualTo, // <=
        kEqualTo, // =
        kNotEqualTo, // !=
        kAnd, // &&
        kOr   // ||
      };

      template<typename T>
      struct VarInfo {
        const std::string name;
        VarInfo( const std::string& in_name ) :name( in_name ) {}
      };

      using VarNameHandel = void*;
      using OperationValues = ::eggs::variant<OperatorE, std::uint64_t, std::int64_t, std::float_t,
        std::double_t, std::uint8_t, std::int8_t, std::string, VarNameHandel>;

      template<OperatorE T>
      struct UnaryOperation : std::false_type {
      };

      template<OperatorE T>
      struct BinaryOperation : std::true_type {
      };
      template<typename T>
      class QueryHelper;

      template<typename T>
      inline std::string value( const VarInfo<T>& in_val ) {
        return in_val::name;
      }

      inline std::string& value( std::string& in_str ) {
        return in_str;
      }

      template<typename T>
      inline T value( const T in_val ) {
        return in_val;
      }
    }
  }
}

/// Query generation
#define GENERATE_VAR_VALUES_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, ",", "")VarInfo(BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define GENERATE_VAR_VALUES(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), GENERATE_VAR_VALUES_I, CLASS_ELEMS_TUP)

#define GENERATE_VAR_NAMES_I(z, n, CLASS_ELEMS_TUP) const static _private::VarNameHandel BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP){reinterpret_cast<_private::VarNameHandel>(n)};
#define GENERATE_VAR_NAMES(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), GENERATE_VAR_NAMES_I, CLASS_ELEMS_TUP)
#define GENERATE_QUERY_INTERFACE(CLASS_ELEMS_TUP) 
namespace blib {
  namespace bun {
    namespace _private {
      template<>
      class QueryHelper<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )> : blib::Singleton<QueryHelper<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )>> {
      public:
        const std::vector<VarInfo<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )>> var_info;
        QueryHelper() :var_info{ GENERATE_VAR_VALUES( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ) ) } {}
      };
    }

    template<>
    class Query<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )> {
    public:
      _private::QueryHelper<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )> _var_mappings;
      std::vector<_private::OperatorE> operators;

    public:
      using VarInfoType = _private::VarInfo<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )>;
      GENERATE_VAR_NAMES( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ) );
      Query() {}
      Query{ Query const& in_other }:_operators( in_other._operators ), _values( in_other._values ){}
      Query( Query&& in_other ) :_operators( std::move( in_other._operators ) ), _values( std::move( in_other._values ) ) {}
    };
  }
}

template<typename T>
blib::bun::Query<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )>& operator&&( blib::bun::Query<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )>& in_left, T const&  in_right ) {
  using QueryType = blib::bun::Query<BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )>;
  static_assert(!(std::is_literal_type<T>::value || std::is_same<T, QueryType>::value), "Need a literal in the RHS");
  if (std::is_same<T, QueryType>::value) {
    in_left.operators.push_back( in_right );
  }
  in_left.operators.push_back( blib::bun::_private::OperatorE::kAnd );

  return in_left;
}



/// CREATE_SCHEMA_TYPES
#define GET_DB_TYPE(CLASS_NAME, ELEMENT) cppTypeToDbTypeString<decltype(CLASS_NAME:: ## ELEMENT)>()
#define CREATE_SCHEMA_TYPES_I(z, n, CLASS_TUP) BOOST_PP_COMMA()BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n, 1), CLASS_TUP))\
BOOST_PP_COMMA() GET_DB_TYPE(BOOST_PP_TUPLE_ELEM(0, CLASS_TUP), BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n, 1), CLASS_TUP))

#define CREATE_SCHEMA_TYPES(CLASS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_TUP), 1), CREATE_SCHEMA_TYPES_I, CLASS_TUP)

/// REPEAT_WITH_COMMA
#define REPEAT_WITH_COMMA_I(z, n, TEXT) BOOST_PP_IF(n, ",", "") TEXT
#define REPEAT_WITH_COMMA(CLASS_TUP, TEXT) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_TUP), REPEAT_WITH_COMMA_I, BOOST_PP_STRINGIZE(TEXT))

/// BunHelper
#define OBJ_TO_STRING_I(z, n, CLASS_ELEMS_TUP) ,in_obj->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP)
#define OBJ_TO_STRING(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), OBJ_TO_STRING_I, CLASS_ELEMS_TUP)

#define UPDATE_STATEMENT_QUERY_I(z, n, CLASS_ELEMS_TUP) ,in_obj->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP)
#define UPDATE_STATEMENT_QUERY(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), UPDATE_STATEMENT_QUERY_I, CLASS_ELEMS_TUP)

#define UPDATE_STATEMENT_QUERY_STR_CAT(ELEM) BOOST_PP_STRINGIZE(ELEM) "= {}"
#define UPDATE_STATEMENT_QUERY_STR_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, ",", "") UPDATE_STATEMENT_QUERY_STR_CAT(BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define UPDATE_STATEMENT_QUERY_STR(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), UPDATE_STATEMENT_QUERY_STR_I, CLASS_ELEMS_TUP)

#define GENERATE_GETOBJ_MEMBERS_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_COMMA_IF(n)ret->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP)
#define GENERATE_GETOBJ_MEMBERS(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), GENERATE_GETOBJ_MEMBERS_I, CLASS_ELEMS_TUP)

#define GENERATE_TABLE_COLUMN_NAMES_PTR_I(z, n, CLASS_ELEMS_TUP) ,in_obj->BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP)
#define GENERATE_TABLE_COLUMN_NAMES_PTR(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), GENERATE_TABLE_COLUMN_NAMES_PTR_I, CLASS_ELEMS_TUP)

#define GENERATE_TABLE_COLUMN_NAMES_I(z, n, CLASS_ELEMS_TUP) ,BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, CLASS_ELEMS_TUP))
#define GENERATE_TABLE_COLUMN_NAMES(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), GENERATE_TABLE_COLUMN_NAMES_I, CLASS_ELEMS_TUP)

#define GENERATE_DB_HELPER(CLASS_ELEMS_TUP) namespace blib{namespace bun{\
template<>\
struct BunHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> {\
using T = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
inline static void createTable(){\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)); \
static const std::string sql = fmt::format( "CREATE TABLE IF NOT EXISTS {}(object_id INTEGER NOT NULL," REPEAT_WITH_COMMA( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {} {} ) ")", \
  class_name CREATE_SCHEMA_TYPES( CLASS_ELEMS_TUP ));\
blib::bun::Db::i().db() << sql;\
}\
\
inline static SimpleOID persistObj( T* in_obj){\
SimpleOID oid;\
oid.populateLow();\
const std::string sql = fmt::format(\
"INSERT INTO {} (object_id," REPEAT_WITH_COMMA( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {} ) ") VALUES({}" REPEAT_WITH_COMMA(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {}) ")",oid.low \
GENERATE_TABLE_COLUMN_NAMES_PTR(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))\
);\
Db::i().db() << sql;\
oid.high = Db::i().db().last_insert_rowid();\
return oid;\
}\
\
inline static void updateObj( T* in_obj, SimpleOID const& in_oid){\
const std::string sql = fmt::format(\
"UPDATE {} SET " UPDATE_STATEMENT_QUERY_STR(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) "WHERE object_id {} AND rowid = {}", BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )) \
UPDATE_STATEMENT_QUERY(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )), in_oid.low, in_oid.high);\
Db::i().db() << sql;\
}\
\
inline static std::unique_ptr<T> getObj( SimpleOID const& in_oid ){\
const std::string sql = fmt::format(\
  "SELECT " REPEAT_WITH_COMMA( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {} ) "FROM {} WHERE object_id = {} AND rowid = {}"\
  GENERATE_TABLE_COLUMN_NAMES( BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ) ), BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )), in_oid.low, in_oid.high \
);\
std::unique_ptr<T> ret( new T );\
Db::i().db() << sql >> std::tie(GENERATE_GETOBJ_MEMBERS(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )));\
return ret;\
}\
\
inline static std::string objToString( T* in_obj, SimpleOID const& in_oid ){\
const std::string ret = fmt::format(\
"{},{},{}" REPEAT_WITH_COMMA(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ), {}), BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM( 0, CLASS_ELEMS_TUP )), in_oid.high, in_oid.low \
OBJ_TO_STRING(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )));\
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
}}

#define REGISTER_CAN_PERSIST(CLASS_NAME) namespace blib{namespace bun{ template<> bool canPersist< CLASS_NAME >(){return true;} } }
#define GENERATE_BINDING(CLASS_ELEMS_TUP) \
REGISTER_CAN_PERSIST(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)) \
GENERATE_DB_HELPER(CLASS_ELEMS_TUP) \
REGISTER_SCHEMA(CLASS_ELEMS_TUP) \
GENERATE_QUERY_INTERFACE(CLASS_ELEMS_TUP)
