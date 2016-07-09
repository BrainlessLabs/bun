#pragma once
#include <boost/proto/proto.hpp>


///------------------------------------------------------------------------
/// Bun Query Start
namespace blib {
  namespace bun {
    namespace query {
      namespace _details {

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
              const auto ret = std::to_string( blib::bun::_details::_E( in_term ) );
              return ret;
            }

            result_type operator ()( boost::proto::tag::terminal, std::string& in_term ) const {
              const auto ret = blib::bun::_details::_E( in_term );
              return ret;
            }

            result_type operator ()( boost::proto::tag::terminal, char const* in_term ) const {
              std::string str = std::string( in_term );
              const auto ret = blib::bun::_details::_E( str );
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

      /////////////////////////////////////////////////
      /// @brief From class for query.
      /////////////////////////////////////////////////
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