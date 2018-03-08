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
#include <set>
#include <type_traits>
#include <boost/mpl/bool.hpp>
#include <boost/preprocessor.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/support/pair.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/preprocessor/tuple/rem.hpp>
#include <third_party/fmt/format.hpp>
#include "blib/bun/PRef.hpp"
//#include "blib/bun/PRefHelper.hpp"
#include "blib/bun/DbBackend.hpp"
#include "blib/bun/DbLogger.hpp"
//#include "blib/bun/QueryHelper.hpp"
#include "blib/bun/CppTypeToSQLString.hpp"
#include "blib/bun/GlobalFunc.hpp"
#include "blib/utils/JSONUtils.hpp"
//#include "blib/bun/NxNMappings.hpp"

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

/// @brief Make this 0 if no log is needed, else make it 1
#define QUERY_LOG_ON 1
/// @brief Log the query
#define QUERY_LOG(log_string) BOOST_PP_EXPR_IF(QUERY_LOG_ON, l().info(log_string))

/// @brief Macro to output comments
#define BLIB_MACRO_COMMENTS_ENABLED 0
#define BLIB_MACRO_COMMENTS_IF(a) BOOST_PP_EXPR_IF(BLIB_MACRO_COMMENTS_ENABLED, a)

/// @brief createSchema Helper Macros
/// @details We need to pass only the data members as a tuple to this macro
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
/// @brief Expands the class members for CREATE TABLE
#define EXPAND_CLASS_MEMBERS_createSchema_I(z, n, ELEMS_TUP) ", " BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)) " {}"
#define EXPAND_CLASS_MEMBERS_createSchema(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_CLASS_MEMBERS_createSchema_I, ELEMS_TUP)

namespace blib {
	namespace bun {
		/////////////////////////////////////////////////
		/// @class IsPersistant
		/// @brief If IsPersistant<Class>::value == true then Class can be persisted.
		///        Specialize this class for all the class which can be persistent.
		/////////////////////////////////////////////////
		template<typename T>
		struct IsPersistant : boost::mpl::bool_<std::is_integral<T>::value> {
		};

		template<>
		struct IsPersistant<std::string> : boost::mpl::bool_<true> {
		};

		namespace __private {
			/////////////////////////////////////////////////
			/// @class DbTableType
			/// @brief Helper for converting primitive types to db types
			/////////////////////////////////////////////////
			template<typename T>
			struct DbTableType {
				using type = T;
			};

			template<typename T>
			struct PrimitiveType {
				blib::bun::SimpleOID oid;
			};
		}
	}
}

namespace blib {
	namespace bun {
		namespace __private {
			/////////////////////////////////////////////////
			/// @class EnclosureType
			/// @brief Holds what kind of mapper type
			/////////////////////////////////////////////////
			enum class EnclosureType {
				kUnknown = 0,
				kBaseType,
				kVector,
				kUniquePointer,
				kSharedPointer
			};

			/////////////////////////////////////////////////
			/// @class FindEncloseeTypeMeta
			/// @brief Deducts the enclosee type
			/////////////////////////////////////////////////				
			template<typename T>
			struct FindEncloseeTypeMeta {
				using type = T;
			};

			template<typename T>
			struct FindEncloseeTypeMeta<std::vector<T>> {
				using type = T;
			};

			template<typename T>
			struct FindEncloseeTypeMeta<std::unique_ptr<T>> {
				using type = T;
			};

			template<typename T>
			struct FindEncloseeTypeMeta<std::shared_ptr<T>> {
				using type = T;
			};

			/////////////////////////////////////////////////
			/// @class IsUniquePointer
			/// @brief Find if this is a unique pointer
			/////////////////////////////////////////////////
			template<typename T>
			struct IsUniquePointer : boost::mpl::bool_<false> {
			};

			template<typename T>
			struct IsUniquePointer<std::unique_ptr<T>> : boost::mpl::bool_<true> {
			};

			/////////////////////////////////////////////////
			/// @class IsSharedPointer
			/// @brief Find if this is a shared pointer
			/////////////////////////////////////////////////
			template<typename T>
			struct IsSharedPointer : boost::mpl::bool_<false> {
			};

			template<typename T>
			struct IsSharedPointer<std::shared_ptr<T>> : boost::mpl::bool_<true> {
			};

			/////////////////////////////////////////////////
			/// @class IsComposite
			/// @brief True if the element is a class/struct that can be persisted
			/////////////////////////////////////////////////
			template<typename T>
			struct IsComposite : boost::mpl::bool_<false> {
			};

			/////////////////////////////////////////////////
			/// @class IsContainer
			/// @brief True if the variable is a container type
			/////////////////////////////////////////////////
			template<typename T>
			struct IsContainer : boost::mpl::bool_<false> {
			};

			template<typename T>
			struct IsContainer<std::vector<T>> : boost::mpl::bool_<true> {
			};

			template<typename T>
			struct IsContainer<std::list<T>> : boost::mpl::bool_<true> {
			};

			template<typename T>
			struct IsContainer<std::set<T>> : boost::mpl::bool_<true> {
			};

			/// @brief Consider a regular pointer as a shared pointer too
			template<typename T>
			struct IsSharedPointer<T*> : boost::mpl::bool_<true> {
			};

			/////////////////////////////////////////////////
			/// @class FindEnclosureTypeMeta
			/// @brief Deducts the enclosing type
			/////////////////////////////////////////////////			
			template<typename T>
			struct FindEnclosureTypeMeta {
				static const EnclosureType = EnclosureType::kBaseType;
			};

			template<typename T>
			struct FindEnclosureTypeMeta<std::vector<T>> {
				static const EnclosureType = EnclosureType::kVector;
			};

			template<typename T>
			struct FindEnclosureTypeMeta<std::unique_ptr<T>> {
				static const EnclosureType = EnclosureType::kUniquePointer;
			};

			template<typename T>
			struct FindEnclosureTypeMeta<std::shared_ptr<T>> {
				static const EnclosureType = EnclosureType::kSharedPointer;
			};

			/////////////////////////////////////////////////
			/// @class TypeDetails
			/// @brief Holds the type details 
			/////////////////////////////////////////////////
			struct TypeDetails {
				blib::bun::DbTypes type;
				EnclosureType enclouser_type;
				bool fundamental_type;

				/// @fn TypeDetails
				TypeDetails(blib::bun::DbTypes in_type,
					EnclosureType in_enclouser_type,
					bool in_fundamental_type) :type(in_type),
					enclouser_type(in_enclouser_type),
					fundamental_type(in_fundamental_type) {}
			};

			/////////////////////////////////////////////////
			/// @class StripQualifiersAndMakePointer
			/// @brief String all the qualifiers like const, volatile, removes reference and adds a pointer.
			/////////////////////////////////////////////////
			template<typename T>
			struct StripQualifiersAndMakePointer {
				using type = std::add_pointer<std::remove_reference<std::remove_cv<T>::type>::type>::type;
			};

			/////////////////////////////////////////////////
			/// @class TypeMetaData
			/// @brief Contains meta data that will be usieful to get type 
			///        inferences for a certain type.
			/////////////////////////////////////////////////
			template<typename T>
			struct TypeMetaData {
				using MT = boost::fusion::vector<void>;
				static std::string const& class_name();
				boost::fusion::vector<boost::fusion::pair<void, std::string>> const& tuple_type_pair();
				static std::map<std::string, TypeDetails> const& type_maps();
			};

			/////////////////////////////////////////////////
			/// @class SqlString
			/// @brief Generates the SQL strings in the class
			/////////////////////////////////////////////////
			template<typename T>
			struct SqlString {
			private:
				static std::string _create_table_sql;
				static std::string _drop_table_sql;
				static std::string _update_row_sql;
				static std::string _insert_row_sql;
				static std::string _delete_row_sql;
				static std::string _select_rows_sql;
				static bool _ok;

				struct SelectRows {
				private:
					std::string& sql;

				public:
					SelectRows(std::string& sql) : sql(sql) {}

					template <typename T>
					void operator()(T const& x) const
					{
						sql += "," + x.second;
					}
				};

				struct CreateTable {
				private:
					std::string& sql;

				public:
					CreateTable(std::string& sql) : sql(sql) {}

					template <typename T>
					void operator()(T const& x) const
					{
						sql += "," + x.second +
							" " +
							blib::bun::cppTypeToDbTypeString<blib::bun::ConvertCPPTypeToSOCISupportType<decltype(std::remove_pointer<T>::type)>::type>();
					}
				};

				struct UpdateRow {
				private:
					std::string& sql;

				public:
					UpdateRow(std::string& sql) : sql(sql) {}

					template <typename T>
					void operator()(T const& x) const
					{
						if (!sql.empty()) {
							sql += ",";
						}
						sql += x.second + " = :" + x.second;
					}
				};

				struct InsertRowNames {
				private:
					std::string& sql;

				public:
					InsertRowNames(std::string& sql) : sql(sql) {}

					template <typename T>
					void operator()(T const& x) const
					{
						sql += ", " + x.second;
					}
				};

				struct InsertRowVal {
				private:
					std::string& sql;

				public:
					InsertRowVal(std::string& sql) : sql(sql) {}

					template <typename T>
					void operator()(T const& x) const
					{
						sql += " , :" + x.second;
					}
				};

			public:
				inline static void populate() {
					if (!_ok) {
						static const auto vecs = TypeMetaData<T>::tuple_type_pair();
						if (_create_table_sql.empty()) {
							_create_table_sql = "CREATE TABLE '{}' IF NOT EXISTS (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER NOT NULL";
							boost::fusion::for_each(vecs, SqlString<T>::CreateTable(_create_table_sql));
							_create_table_sql += ")"
						}

						if (_drop_table_sql.empty()) {
							_drop_table_sql = "DROP TABLE '{}'";
						}

						if (_delete_row_sql.empty()) {
							_delete_row_sql = "DELETE FROM '{}' WHERE oid_high = {} AND oid_low = {}";
						}

						if (_insert_row_sql.empty()) {
							_insert_row_sql = "INSERT INTO '{}' (oid_high, oid_low";
							boost::fusion::for_each(vecs, SqlString<T>::InsertRowNames(_insert_row_sql));
							_insert_row_sql += ") VALUES (:oid_high, :oid_low";
							boost::fusion::for_each(vecs, SqlString<T>::InsertRowVal(_insert_row_sql));
							_insert_row_sql += ")";
						}

						if (_update_row_sql.empty()) {
							_update_row_sql = "UPDATE '{}' SET ";
							std::string sql;
							boost::fusion::for_each(vecs, SqlString<T>::UpdateRow(sql));
							_update_row_sql += sql + " WHERE oid_high = :oid_high AND oid_low = :oid_low";
						}

						if (_select_rows_sql.empty()) {
							_select_rows_sql = "SELECT oid_high, oid_low";
							boost::fusion::for_each(vecs, SqlString<T>::SelectRows(_select_rows_sql));
							_select_rows_sql += "FROM '{}' ";
						}
						_ok = true;
					}
				}

				static bool const ok() {
					return _ok;
				}

				static std::string const& create_table_sql() {
					return _create_table_sql;
				}

				static std::string const& drop_table_sql() {
					return _drop_table_sql;
				}

				static std::string const& delete_row_sql() {
					return _delete_row_sql;
				}

				static std::string const& insert_row_sql() {
					return _insert_row_sql;
				}

				static std::string const& update_row_sql() {
					return _update_row_sql;
				}
			};

			/////////////////////////////////////////////////
			/// @class PRefHelper
			/// @brief Helper class for the persistent framework.
			///        This class is specialized to persist objects.
			/////////////////////////////////////////////////
			template<typename T>
			struct QueryHelper {
				static bool _ok;
				static std::string _create_table_sql;
				static std::string _drop_table_sql;
				static std::string _update_row_sql;
				static std::string _table_name;

				inline static void createSchema() {
					const static std::string sql = fmt::format(SqlString<T>::create_table_sql(), TypeMetaData<T>::class_name());
					QUERY_LOG(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const & e) {
						l().error("createSchema: {} ", e.what());
					}
				}

				inline static void createSchema(const std::string& parent_table) {
					const std::string sql = fmt::format(SqlString<T>::create_table_sql(), parent_table + "_" + TypeMetaData<T>::class_name());
					QUERY_LOG(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const & e) {
						l().error("createSchema({}): {} ", parent_table, e.what());
					}
				}

				inline static void deleteSchema() {
					const static std::string sql = fmt::format(SqlString<T>::drop_table_sql(), TypeMetaData<T>::class_name());
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const & e) {
						l().error("deleteSchema(): {} ", e.what());
					}
				}

				inline static void deleteSchema(const std::string& parent_table) {
					const std::string sql = fmt::format(SqlString<T>::drop_table_sql(), parent_table + "_" + TypeMetaData<T>::class_name());
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const & e) {
						l().error("deleteSchema({}): {} ", parent_table, e.what());
					}
				}

				inline static SimpleOID persistObj(T *obj) {
				}

				inline static void updateObj(T *, SimpleOID const &);

				inline static void deleteObj(SimpleOID const &);

				inline static std::unique_ptr <T> getObj(SimpleOID const &);

				inline static std::string md5(T *, SimpleOID const &);

				inline static std::string objToString(T *, SimpleOID const &);

				inline static std::string objToJson(T *, SimpleOID const &);
			};
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros Start
///////////////////////////////////////////////////////////////////////////////

/// @brief SOCI ORM Helper Macros
/// @details from_base
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
#define EXPAND_MEMBER_ASSIGNENTS_from_base_I(z, n, ELEMS_TUP) c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP) = v.get<blib::bun::__private::ConvertCPPTypeToSOCISupportType<decltype(c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)));
#define EXPAND_MEMBER_ASSIGNENTS_from_base(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_MEMBER_ASSIGNENTS_from_base_I, ELEMS_TUP)

/// @details to_base
/// @param ELEMS_TUP = (bun_name, sugar_quantity, flour_quantity, milk_quantity, yeast_quantity, butter_quantity, bun_length)
#define EXPAND_MEMBER_ASSIGNENTS_to_base_I(z, n, ELEMS_TUP) v.set(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)), blib::bun::__private::convertToSOCISupportedType(c.BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)));
#define EXPAND_MEMBER_ASSIGNENTS_to_base(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_MEMBER_ASSIGNENTS_to_base_I, ELEMS_TUP)

/// @details type_maps
#define EXPAND_MEMBER_ASSIGNENTS_generate_type_maps_I(z, n, CLASS_ELEMS_TUP) {\
BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)), \
TypeDetails(\
blib::bun::CppTypeToDbType<FindEncloseeTypeMeta<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)>::type>::ret,\
FindEnclosureTypeMeta<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)>::type,\
std::is_fundamental<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n)>::value\
)\
},
#define EXPAND_MEMBER_ASSIGNENTS_generate_type_maps(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), EXPAND_MEMBER_ASSIGNENTS_generate_type_maps_I, CLASS_ELEMS_TUP)

#define GENERATE_TupType_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupType_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<blib::bun::__private::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePair_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<blib::bun::__private::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS_TUP)
///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

/// SPECIALIZE_BUN_HELPER Start
#define SPECIALIZE_BUN_HELPER(CLASS_ELEMS_TUP) BOOST_FUSION_ADAPT_STRUCT(\
BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS_TUP)\
)\
namespace blib{namespace bun{\
template<>\
BLIB_MACRO_COMMENTS_IF("@brief Lets everyone know that this is a persistant class");\
struct IsPersistant<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> : boost::mpl::bool_<true> {\
};\
BLIB_MACRO_COMMENTS_IF("@brief Mark the class as composite");\
template<>\
struct CppTypeToDbType<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
static const DbTypes ret = DbTypes::kComposite;\
};\
}\
namespace blib{namespace bun{namespace __private{\
template<>\
struct IsComposite<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> : boost::mpl::bool_<true> {\
};\
template<>\
TypeMetaData<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
using TupType = boost::fusion::tuple<GENERATE_TupType(CLASS_ELEMS_TUP)>;\
using TupTypePairType = boost::fusion::tuple<GENERATE_TupTypePair(CLASS_ELEMS_TUP)>;\
using T = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
static auto tuple_type_pair()->TupTypePairType const&{\
static const TupTypePairType t{GENERATE_TupTypePairObj(CLASS_ELEMS_TUP)};\
return t;\
}\
static std::string const& class_name(){\
static std::string const class_name = BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
return class_name;\
}\
static std::map<std::string, TypeDetails> const& type_maps(){\
static const std::map<std::string, TypeDetails> type_map = {\
EXPAND_MEMBER_ASSIGNENTS_generate_type_maps(CLASS_ELEMS_TUP);\
};\
return type_map;\
}\
};\
}}}\
namespace soci{\
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
/// SPECIALIZE_BUN_HELPER End
