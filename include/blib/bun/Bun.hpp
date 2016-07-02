#pragma once

#include <hdr/sqlite_modern_cpp.hpp>
#include <boost/preprocessor.hpp>
#include <boost/proto/proto.hpp>
#include <fmt/format.hpp>
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
#include <vector>
#include <type_traits>

namespace blib {
  namespace bun {
    inline spdlog::logger& l() {
      static const std::size_t q_size = 1048576; //queue size must be power of 2
      spdlog::set_async_mode( q_size );
      static auto ret = spdlog::daily_logger_st( "async_file_logger", "query_log.txt" );
      return *ret;
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

    inline void dbConnect( std::string const& in_db ) {
      Db::i().connect( in_db );
    }

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

      void clear() {
        high = 0;
        low = 0;
      }

      SelfType& operator=( SelfType const& in_other ) {
        high = in_other.high;
        low = in_other.low;
        return *this;
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
      PRef( PRef const& in_other ) : oid( in_other.oid ) {
        load( oid );
      }

      PRef( PRef& in_other ) : oid( in_other.oid ), _flags( in_other._flags ), _md5( in_other._md5 ), _obj( in_other._obj.release() ) {
      }

      PRef( ObjType* in_obj ) :_obj( in_obj ) {
      }

      PRef( OidType const& in_oid ) : oid( in_oid ) {
        load( oid );
      }

      PRef( OidType const& in_oid, ObjType* in_obj ) : oid( in_oid ), _obj( in_obj ) {
      }

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

      decltype(*_obj) operator*() {
        return *_obj;
      }

      T* operator->() {
        return _obj.get();
      }

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
          BunHelper<ObjType>::updateObj( _obj.get(), oid );
        }
        _md5 = BunHelper<ObjType>::md5( _obj.get(), oid );
        _flags.reset();
        return oid;
      }

      OidType save() {
        return persist();
      }

      void del() {
        BunHelper<ObjType>::deleteObj( oid );
        _md5.clear();
        oid.clear();
        _flags.reset();
      }

      PRef& operator=( ObjType* in_obj ) {
        reset( in_obj );
        return *this;
      }

      PRef& operator=( PRef& in_other ) {
        copyFrom( in_other );
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

    private:
      void load( OidType const& in_oid ) {
        oid = in_oid;
        _obj = BunHelper<ObjType>::getObj( oid );
        _md5 = BunHelper<ObjType>::md5( _obj.get(), oid );
        _flags.reset();
      }

      void copyFrom( PRef& in_other ) {
        oid = in_other.oid;
        _md5 = in_other._md5;
        _flags = in_other._flags;
        _obj = in_other._obj;
      }
    };

    namespace _details {
      template<typename T>
      struct BoxingType;

      template<>
      struct BoxingType<int> {
        using type = std::int64_t;
      };

      template<>
      struct BoxingType<unsigned int> {
        using type = std::uint64_t;
      };

      template<>
      struct BoxingType<float> {
        using type = double;
      };

      template<>
      struct BoxingType<double> {
        using type = double;
      };

      template<>
      struct BoxingType<char> {
        using type = std::int64_t;
      };

      template<>
      struct BoxingType<unsigned char> {
        using type = std::uint64_t;
      };

      template<>
      struct BoxingType<std::string> {
        using type = std::string;
      };

      /// Method to put quote around strings. This is used for the formation of the Sql String
      template<typename T>
      inline T _E( const T in_val ) {
        return in_val;
      }

      inline std::string& _E( std::string& in_val ) {
        in_val = "'" + in_val + "'";
        return in_val;
      }
    }

    template<typename T>
    inline std::vector<PRef<T>> getAllObjects();
  }
}

///------------------------------------------------------------------------
/// Bun Query Start
namespace blib {
  namespace bun {
    namespace query {
      namespace _details {
        template<typename T>
        inline std::vector<blib::bun::PRef<T>> getAllObjWithQuery( std::string const& in_query );

        template<typename T>
        inline std::string const& className();

        template<std::int32_t I>
        struct QueryVariablePlaceholderIndex : std::integral_constant <std::int32_t, I> {
        };

        // Grammar for the query Start
        struct PlaceHoldersTerminals : boost::proto::or_ <
          boost::proto::terminal<QueryVariablePlaceholderIndex<0>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<1>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<2>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<3>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<4>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<5>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<6>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<7>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<8>>,
          boost::proto::or_<
          boost::proto::terminal<QueryVariablePlaceholderIndex<9>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<10>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<11>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<12>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<13>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<14>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<15>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<16>>,
          boost::proto::terminal<QueryVariablePlaceholderIndex<17>>
          >
        > {
        };

        struct QueryLiteralTerminals : boost::proto::or_<
          boost::proto::terminal<boost::proto::convertible_to<int>>,
          boost::proto::terminal<boost::proto::convertible_to<unsigned int>>,
          boost::proto::terminal<boost::proto::convertible_to<char>>,
          boost::proto::terminal<boost::proto::convertible_to<unsigned char>>,
          boost::proto::terminal<boost::proto::convertible_to<float>>,
          boost::proto::terminal<boost::proto::convertible_to<double>>,
          boost::proto::terminal<boost::proto::convertible_to<bool>>,
          boost::proto::terminal<boost::proto::convertible_to<std::string>>
        > {
        };

        struct AllTerminals : boost::proto::or_<
          PlaceHoldersTerminals,
          QueryLiteralTerminals
        > {
        };

        struct GreaterSymbols : boost::proto::or_<
          boost::proto::greater<AllTerminals, AllTerminals>,
          boost::proto::greater_equal<AllTerminals, AllTerminals>
        > {
        };

        struct LessSymbols : boost::proto::or_<
          boost::proto::less<AllTerminals, AllTerminals>,
          boost::proto::less_equal<AllTerminals, AllTerminals>
        > {
        };

        struct EqualSymbols : boost::proto::or_<
          boost::proto::equal_to<AllTerminals, AllTerminals>,
           boost::proto::not_equal_to<AllTerminals, AllTerminals>
        > {
        };

        struct BunQueryGrammar : boost::proto::or_<
          AllTerminals,
          GreaterSymbols,
          LessSymbols,
          EqualSymbols,
          boost::proto::logical_and<BunQueryGrammar, BunQueryGrammar>,
          boost::proto::logical_or<BunQueryGrammar, BunQueryGrammar>
        > {
        };

        // Grammar for the query End
        template<typename T>
        inline std::string const& mapping( const std::uint32_t in_index );
        //{
        //  static const std::vector<std::string> ret = { "name", "age", "height" };
        //  return ret.at( in_index );
        //}

        template<typename T>
        struct TypesUsed;
        //boost::mpl::vector<decltype(test::Person::name), decltype(test::Person::age), decltype(test::Person::height)>;

        template<typename T>
        struct FromInternals {
          using TypesUsed = typename TypesUsed<T>::Type;

          struct BunQueryFilterContex : boost::proto::callable_context<BunQueryFilterContex> {
            typedef std::string result_type;
            BunQueryFilterContex() {}

            template<typename T>
            result_type operator ()( boost::proto::tag::terminal, T in_term ) const {
              const auto ret = std::to_string( blib::bun::_details::_E(in_term) );
              return ret;
            }

            result_type operator ()( boost::proto::tag::terminal, std::string& in_term ) const {
              const auto ret = blib::bun::_details::_E(in_term);
              return ret;
            }

            result_type operator ()( boost::proto::tag::terminal, char const* in_term ) const {
              std::string str = std::string( in_term );
              const auto ret = blib::bun::_details::_E(str);
              return ret;
            }

            template<std::uint32_t I>
            result_type operator()( boost::proto::tag::terminal, bun::query::_details::QueryVariablePlaceholderIndex<I> in_term ) const {
              const auto ret = mapping<T>( I );
              return ret;
            }

            template<typename L, typename R>
            result_type operator()( boost::proto::tag::logical_and, L const& in_l, R const& in_r ) const {
              static const std::string operator_name = " AND ";
              auto ctx = *this;
              const auto left_string = boost::proto::eval( in_l, ctx );
              const auto right_string = boost::proto::eval( in_r, ctx );
              const std::string ret = left_string + operator_name + right_string;
              return ret;
            }

            template<typename L, typename R>
            result_type operator()( boost::proto::tag::logical_or, L const& in_l, R const& in_r ) const {
              static const std::string operator_name = " OR ";
              auto ctx = *this;
              const auto left_string = boost::proto::eval( in_l, ctx );
              const auto right_string = boost::proto::eval( in_r, ctx );
              const std::string ret = left_string + operator_name + right_string;
              return ret;
            }

            template<typename L, typename R>
            result_type operator()( boost::proto::tag::less, L const& in_l, R const& in_r ) const {
              static const std::string operator_name = " < ";
              auto ctx = *this;
              const auto left_string = boost::proto::eval( in_l, ctx );
              const auto right_string = boost::proto::eval( in_r, ctx );
              const std::string ret = left_string + operator_name + right_string;
              return ret;
            }

            template<typename L, typename R>
            result_type operator()( boost::proto::tag::less_equal, L const& in_l, R const& in_r ) const {
              static const std::string operator_name = " <= ";
              auto ctx = *this;
              const auto left_string = boost::proto::eval( in_l, ctx );
              const auto right_string = boost::proto::eval( in_r, ctx );
              const std::string ret = left_string + operator_name + right_string;
              return ret;
            }

            template<typename L, typename R>
            result_type operator()( boost::proto::tag::greater, L const& in_l, R const& in_r ) const {
              static const std::string operator_name = " > ";
              auto ctx = *this;
              const auto left_string = boost::proto::eval( in_l, ctx );
              const auto right_string = boost::proto::eval( in_r, ctx );
              const std::string ret = left_string + operator_name + right_string;
              return ret;
            }

            template<typename L, typename R>
            result_type operator()( boost::proto::tag::greater_equal, L const& in_l, R const& in_r ) const {
              static const std::string operator_name = " >= ";
              auto ctx = *this;
              const auto left_string = boost::proto::eval( in_l, ctx );
              const auto right_string = boost::proto::eval( in_r, ctx );
              const std::string ret = left_string + operator_name + right_string;
              return ret;
            }

            template<typename L, typename R>
            result_type operator()( boost::proto::tag::equal_to, L const& in_l, R const& in_r ) const {
              static const std::string operator_name = " == ";
              auto ctx = *this;
              const auto left_string = boost::proto::eval( in_l, ctx );
              const auto right_string = boost::proto::eval( in_r, ctx );
              const std::string ret = left_string + operator_name + right_string;
              return ret;
            }

            template<typename L, typename R>
            result_type operator()( boost::proto::tag::not_equal_to, L const& in_l, R const& in_r ) const {
              static const std::string operator_name = " != ";
              auto ctx = *this;
              const auto left_string = boost::proto::eval( in_l, ctx );
              const auto right_string = boost::proto::eval( in_r, ctx );
              const std::string ret = left_string + operator_name + right_string;
              return ret;
            }

            template<typename TerminalType, typename L, typename R>
            result_type operator()( TerminalType, L const& in_l, R const& in_r ) const {
              static_assert(false, "Operator not supported in Bun");
              const std::string ret = "Operator not supported in Bun";
              return ret;
            }

            template<typename TerminalType, typename L>
            result_type operator()( TerminalType, L const& in_l ) const {
              static_assert(false, "Operator not supported in Bun");
              const std::string ret = "Operator not supported in Bun";
              return ret;
            }
          };
        };
      }

      // Fields for query
      namespace {
        template<typename T>
        struct F;
      }

      template<typename T>
      struct From {
      private:
        std::string _query;
        decltype(_details::getAllObjWithQuery<T>( "" )) _objects;

      private:
        template<typename ExpressionType>
        std::string eval( ExpressionType const& in_expr ) {
          _details::FromInternals<T>::BunQueryFilterContex ctx;
          const std::string ret = boost::proto::eval( in_expr, ctx );
          return ret;
        }

        static std::string const& className() {
          static const auto table_name = className<T>();
          return table_name;
        }

      public:
        From() = default;

        From( From& in_other ) :_query( in_other._query ), _objects( in_other._objects ) {}

        template<typename ExpressionType>
        From& where( ExpressionType const& in_expr ) {
          static_assert(boost::proto::matches<ExpressionType, _details::BunQueryGrammar>::value, "Syntax error in Bun Query");
          const std::string query_string = eval( in_expr );
          const std::string add_string = _query.empty() ? "" : " AND ";
          _query += add_string + query_string;
          return *this;
        }

        std::string const& query() const {
          l().info() << _query;
          return _query;
        }

        decltype(_objects)& objects() {
          _objects = _details::getAllObjWithQuery<T>( _query );
          return _objects;
        }
      };

      template<typename ExpressionType>
      struct IsValidQuery : std::integral_constant<bool, boost::proto::matches<ExpressionType, _details::BunQueryGrammar>::value> {
      };
    }
  }
}
/// Bun Query End
///------------------------------------------------------------------------

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
#define REGISTER_CAN_PERSIST(CLASS_NAME) namespace blib{namespace bun{ template<> inline bool canPersist< CLASS_NAME >(){return true;} } }

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
const static std::string sql = fmt::format(\
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
