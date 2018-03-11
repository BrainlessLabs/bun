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
#include <cstdint>
#include <cstddef>
#include <memory>
#include <bitset>
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
//#include "blib/bun/PRef.hpp"
#include "blib/utils/MD5.hpp"
//#include "blib/bun/PRefHelper.hpp"
#include "blib/bun/DbBackend.hpp"
#include "blib/bun/DbLogger.hpp"
#include "blib/bun/SimpleOID.hpp"
//#include "blib/bun/QueryHelper.hpp"
#include "blib/bun/CppTypeToSQLString.hpp"
//#include "blib/bun/GlobalFunc.hpp"
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
			/// @class StripQualifiersAndMakePointer
			/// @brief String all the qualifiers like const, volatile, removes reference and adds a pointer.
			/////////////////////////////////////////////////
			template<typename T>
			struct StripQualifiersAndMakePointer {
				using no_cv_type = typename std::remove_cv<T>::type;
				using no_cv_no_ref = typename std::remove_reference<no_cv_type>::type;
				using type = typename std::add_pointer<no_cv_no_ref>::type;
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
			};

			/////////////////////////////////////////////////
			/// @class SqlString
			/// @brief Generates the SQL strings in the class
			/////////////////////////////////////////////////
			template<typename T>
			struct SqlString {
			private:
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
						using ObjType = std::remove_const<std::remove_pointer<typename T::first_type>::type>::type;
						sql += "," + x.second +
							" " +
							blib::bun::cppTypeToDbTypeString<blib::bun::__private::ConvertCPPTypeToSOCISupportType<ObjType>::type>();
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
				static std::string const& create_table_sql() {
					static const auto vecs = TypeMetaData<T>::tuple_type_pair();
					static std::string sql;
					if (sql.empty()) {
						sql = "CREATE TABLE IF NOT EXISTS '{}' (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER NOT NULL";
						boost::fusion::for_each(vecs, SqlString<T>::CreateTable(sql));
						sql += ")";
					}
					return sql;
				}

				static std::string const& drop_table_sql() {
					static const auto vecs = TypeMetaData<T>::tuple_type_pair();
					static std::string sql;
					if (sql.empty()) {
						sql = "DROP TABLE '{}'";
					}
					return sql;
				}

				static std::string const& delete_row_sql() {
					static const auto vecs = TypeMetaData<T>::tuple_type_pair();
					static std::string sql;
					if (sql.empty()) {
						sql = "DELETE FROM '{}' WHERE oid_high = :oid_high AND oid_low = :oid_low";
					}
					return sql;
				}

				static std::string const& insert_row_sql() {
					static const auto vecs = TypeMetaData<T>::tuple_type_pair();
					static std::string sql;
					if (sql.empty()) {
						sql = "INSERT INTO '{}' (oid_low";
						boost::fusion::for_each(vecs, SqlString<T>::InsertRowNames(sql));
						sql += ") VALUES ({}";
						boost::fusion::for_each(vecs, SqlString<T>::InsertRowVal(sql));
						sql += ")";
					}
					return sql;
				}

				static std::string const& update_row_sql() {
					static const auto vecs = TypeMetaData<T>::tuple_type_pair();
					static std::string sql;
					if (sql.empty()) {
						sql = "UPDATE '{}' SET ";
						std::string sql1;
						boost::fusion::for_each(vecs, SqlString<T>::UpdateRow(sql1));
						sql += sql1 + " WHERE oid_high = {} AND oid_low = {}";
					}
					return sql;
				}

				static std::string const& select_rows_sql() {
					static const auto vecs = TypeMetaData<T>::tuple_type_pair();
					static std::string sql;
					if (sql.empty()) {
						sql = "SELECT oid_high, oid_low";
						boost::fusion::for_each(vecs, SqlString<T>::SelectRows(sql));
						sql += " FROM '{}' ";
					}
					return sql;
				}
			};

			/////////////////////////////////////////////////
			/// @class SimpleObjHolder
			/// @brief A simple holder for objects. 
			/// 	   To be used for the from_base and to_base conversion.
			///		   To be used for the orm object mapping
			/////////////////////////////////////////////////			
			template<typename T>
			struct SimpleObjHolder {
				T* obj;
				blib::bun::SimpleOID& oid;
				SimpleObjHolder(T* obj_in, blib::bun::SimpleOID& oid_in) :obj(obj_in), oid(oid_in) {}
			};

			/////////////////////////////////////////////////
			/// @class PRefHelper
			/// @brief Helper class for the persistent framework.
			///        This class is specialized to persist objects.
			/////////////////////////////////////////////////
			template<typename T>
			struct QueryHelper {
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
					QUERY_LOG(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const & e) {
						l().error("deleteSchema(): {} ", e.what());
					}
				}

				inline static void deleteSchema(const std::string& parent_table) {
					const std::string sql = fmt::format(SqlString<T>::drop_table_sql(), parent_table + "_" + TypeMetaData<T>::class_name());
					QUERY_LOG(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const & e) {
						l().error("deleteSchema({}): {} ", parent_table, e.what());
					}
				}

				inline static SimpleOID persistObj(T *obj) {
					blib::bun::SimpleOID oid;
					oid.populateLow();
					const static std::string sql = fmt::format(SqlString<T>::insert_row_sql(), TypeMetaData<T>::class_name(), oid.low);
					//SimpleObjHolder obj_holder(obj, oid);
					QUERY_LOG(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql, soci::use(*obj);
						long high = 0;
						if (blib::bun::__private::DbBackend<>::i().session().get_last_insert_id(TypeMetaData<T>::class_name(), high)) {
							oid.high = static_cast<decltype(oid.high)>(high);
						}
					}
					catch (std::exception const & e) {
						l().error("persistObj(): {} ", e.what());
					}
					return std::move(oid);
				}

				inline static void updateObj(T * obj, SimpleOID const & oid) {
					const static std::string sql = fmt::format(SqlString<T>::update_row_sql(), TypeMetaData<T>::class_name(), oid.high, oid.low);
					QUERY_LOG(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql, soci::use(*obj);
					}
					catch (std::exception const & e) {
						l().error("updateObj(): {} ", e.what());
					}
				}

				inline static void deleteObj(SimpleOID const & oid) {
					const static std::string sql = fmt::format(SqlString<T>::delete_row_sql(), TypeMetaData<T>::class_name());
					QUERY_LOG(sql);
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql, soci::use(oid.high), soci::use(oid.low);
					}
					catch (std::exception const & e) {
						l().error("deleteObj(): {} ", e.what());
					}
				}

				inline static std::unique_ptr <T> getObj(SimpleOID const & oid) {
					const static std::string sql = fmt::format(SqlString<T>::select_rows_sql() + " WHERE oid_high = {} AND oid_low = {}", TypeMetaData<T>::class_name(), oid.high, oid.low);
					QUERY_LOG(sql);
					std::unique_ptr <T> obj = std::make_unique<T>();
					try {
						blib::bun::__private::DbBackend<>::i().session() << sql;
					}
					catch (std::exception const & e) {
						l().error("getObj(): {} ",  e.what());
					}
					return std::move(obj);
				}

				inline static std::string md5(T * obj, SimpleOID const & oid) {
					const std::string str = QueryHelper<T>::objToJson(obj, oid);
					const std::string md5 = blib::md5(str);
					return std::move(md5);
				}

				inline static std::string objToString(T * obj, SimpleOID const & oid) {
					return QueryHelper<T>::objToJson(obj, oid);
				}

				struct ToJson {
				private:
					std::string& str;

				public:
					ToJson(std::string & str) :str(str) {}

					template <typename T>
					void operator()(T const& x) const
					{
						str += fmt::format("'f': {}", x);
					}
				};

				inline static std::string objToJson(T * obj, SimpleOID const & oid) {
					T& v = *obj;
					std::string str = fmt::format("'oid_high': {}, 'oid_high': {}", oid.high, oid.low);
					boost::fusion::for_each(v, QueryHelper<T>::ToJson(str));
					str += "{" + str + "}";
					return std::move(str);
				}

				struct GetAllObjects {
				private:
					const soci::row& _row;
					const std::vector<std::string>& _member_names;
					int _count;

				public:
					GetAllObjects(const soci::row& row) :_row(row), _member_names(TypeMetaData<T>::member_names()), _count(0) {}

					template <typename T>
					void operator()(T& x)
					{
						x = _row.get<ConvertCPPTypeToSOCISupportType<T>::type>(_member_names.at(_count));
						++_coun;
					}
				};

				inline static std::vector<std::pair<std::unique_ptr <T>, SimpleOID>> getAllObjectsWithQuery(const std::string&& in_query = std::string()) {
					std::vector<std::pair<std::unique_ptr <T>, SimpleOID>> ret_values;

					const static std::string select_sql = fmt::format(SqlString<T>::select_rows_sql(), TypeMetaData<T>::class_name()) + " {}";
					const std::string where_clasue = in_query.empty() ? "" : "WHERE " + in_query;
					const std::string sql = fmt::format(select_sql, where_clasue);
					QUERY_LOG(sql);
					try {
						soci::rowset<soci::row> rows = (blib::bun::__private::DbBackend<>::i().session().prepare << sql);
						for (soci::rowset<soci::row>::const_iterator row_itr = rows.begin(); row_itr != rows.end(); ++row_itr) {
							auto const& row = *row_itr;
							std::pair<std::unique_ptr <T>, SimpleOID> pair;
							pair.second.high = row.get<ConvertCPPTypeToSOCISupportType<SimpleOID::OidHighType>::type>("oid_high");
							pair.second.low = row.get<ConvertCPPTypeToSOCISupportType<SimpleOID::OidLowType>::type>("oid_low");
							pair.first = std::make_unique<T>();
							T& obj = *pair.first;
							boost::fusion::for_each(obj, QueryHelper<T>::GetAllObjects(row));
							ret_values.push_back(pair);
						}
					}
					catch (std::exception const & e) {
						l().error("getAllObjectsWithQuery(""): {} ", e.what());
					}
					return std::move(ret_values);
				}
			};
		}
	}
}

namespace blib {
	namespace bun {
		/////////////////////////////////////////////////
		/// @class PRef
		/// @brief The persistent reference holder.
		/// @details This is the primary object holder.
		///          Anything assigned to this can be stored in the database.
		/////////////////////////////////////////////////
		template<typename T>
		class PRef {
		private:
			enum class FlagsE : std::uint8_t {
				kDirty = 0
			};

			/// @typedef OidType = SimpleOID
			using OidType = SimpleOID;
			/// @var std::unique_ptr<T> _obj
			/// @brief Stores the object. The unique pointer is specialized
			///        for that object type.
			std::unique_ptr<T> _obj;
			/// @var  std::bitset<4> _flags
			std::bitset<4> _flags;
			/// @var std::string _md5
			/// @brief Holds the MD5 sum of this object.
			std::string _md5;

		public:
			using ObjType = T;
			typedef PRef<T> SelfType;
			/// @var OidType oid
			/// @brief Holds the OID for this object.
			///        Each object will have an unique OID.
			///        This will distinguish them from other object.
			OidType oid;

		public:
			PRef() = default;

			PRef(PRef const &in_other) : oid(in_other.oid) {
				load(oid);
			}

			PRef(PRef &in_other) : oid(in_other.oid), _flags(in_other._flags), _md5(in_other._md5),
				_obj(in_other._obj.release()) {
			}

			PRef(ObjType *in_obj) : _obj(in_obj) {
			}

			PRef(OidType const &in_oid) : oid(in_oid) {
				load(oid);
			}

			PRef(OidType const &in_oid, ObjType *in_obj) : oid(in_oid), _obj(in_obj) {
			}

			/// @fm reset
			/// @brief Resets the current PRef and assigns another object to it.
			/// @param in_obj The other object to assign it to.
			void reset(ObjType *in_obj) {
				_obj.reset(in_obj);
				_flags.reset();
				_md5 = "";
			}

			/// @fn release
			/// @brief Reset the PRef to hold nothing.
			///        The oid is set to 0 and the object released.
			ObjType *release() {
				oid.high = 0;
				oid.low = 0;
				_flags.reset();
				_md5 = "";
				return _obj.release();
			}

			~PRef() = default;

			auto operator*()->decltype(*_obj) {
				return *_obj;
			}

			T *operator->() {
				return _obj.get();
			}

			/// @fn dirty
			/// @brief Returns true if the object is changed from last commit
			///        else false.
			/// @details The MD5 of the object is taken from the last commit.
			///          If the MD5 are different then it returns true else
			///          it returns false.
			bool dirty() {
				const auto md5 = blib::bun::__private::QueryHelper<ObjType>::md5(_obj.get(), oid);
				if (md5 != _md5) {
					_flags[static_cast<std::uint8_t>(FlagsE::kDirty)] = 1;
				}

				return _flags[static_cast<std::uint8_t>(FlagsE::kDirty)] ? true : false;
			}

			/// @fn persist
			/// @brief Commits the object in database
			/// @details Commits the object in database and also updates the
			///          MD5 of the object. If this function is not called
			///          the object is not going to be updated in database.
			/// @return OidType Returns the OID of the persisted object.
			OidType persist() {
				if (_md5.empty()) {
					oid = blib::bun::__private::QueryHelper<ObjType>::persistObj(_obj.get());
				}
				else {
					blib::bun::__private::QueryHelper<ObjType>::updateObj(_obj.get(), oid);
				}
				_md5 = blib::bun::__private::QueryHelper<ObjType>::md5(_obj.get(), oid);
				_flags.reset();
				return oid;
			}

			/// @fn save
			/// @brief Does same as persist. Calls persist internally.
			OidType save() {
				return persist();
			}

			/// @fn del
			/// @brief Delets the persistent object.
			///        Clears the MD5 and the flags.
			void del() {
				blib::bun::__private::QueryHelper<ObjType>::deleteObj(oid);
				_md5.clear();
				oid.clear();
				_flags.reset();
			}

			PRef &operator=(ObjType *in_obj) {
				reset(in_obj);
				return *this;
			}

			PRef &operator=(PRef &in_other) {
				copyFrom(in_other);
				return *this;
			}

			bool operator==(PRef const &in_other) {
				return oid == in_other.oid;
			}

			bool operator!=(PRef const &in_other) {
				return oid != in_other.oid;
			}

			/// @fn toJson
			/// @brief Returns a JSON representation of the object.
			std::string toJson() const {
				return blib::bun::__private::QueryHelper<T>::objToJson(_obj.get(), oid);
			}

		private:
			/// @fn oad(OidType const &in_oid)
			/// @brief Loads an object from Database.
			/// @param in_oid A valid OID for the object.
			void load(OidType const &in_oid) {
				oid = in_oid;
				_obj = blib::bun::__private::QueryHelper<ObjType>::getObj(oid);
				_md5 = blib::bun::__private::QueryHelper<ObjType>::md5(_obj.get(), oid);
				_flags.reset();
			}

			/// @fn copyFrom(PRef &in_other)
			/// @brief Create a exact clone of another PRef
			void copyFrom(PRef &in_other) {
				oid = in_other.oid;
				_md5 = in_other._md5;
				_flags = in_other._flags;
				_obj = in_other._obj;
			}
		};
	}
}

namespace blib {
	namespace bun {
		/////////////////////////////////////////////////
		/// @brief Helper class for the persistent framework.
		///        This class is specialized to persist objects.
		/////////////////////////////////////////////////

		/// @fn createSchema
		/// @brief Create the schema for the object
		template<typename T>
		inline static void createSchema() {
			soci::transaction t(blib::bun::__private::DbBackend<>::i().session());
			blib::bun::__private::QueryHelper<T>::createSchema();
			t.commit();
		}

		/// @fn deleteSchema
		/// @brief Delete the schema for the object
		template<typename T>
		inline static void deleteSchema() {
			soci::transaction t(blib::bun::__private::DbBackend<>::i().session());
			blib::bun::__private::QueryHelper<T>::deleteSchema<T>();
			t.commit();
		}

		template<typename T>
		inline static std::vector <SimpleOID> getAllOids() {
			//soci::transaction t(blib::bun::__private::DbBackend<>::i().session());
			return blib::bun::__private::QueryHelper<T>::getAllOids();
			//t.commit();
		}

		template<typename T>
		inline static std::vector <PRef<T>> getAllObjects() {
			//soci::transaction t(blib::bun::__private::DbBackend<>::i().session());
			return getAllObjWithQuery<T>();
			//t.commit();
		}

		template<typename T>
		inline static std::vector <PRef<T>> getAllObjWithQuery(std::string const &in_query) {
			const auto values = blib::bun::__private::QueryHelper<T>::getAllObjWithQuery(in_query);
			std::vector <PRef<T>> ret_vals;
			for (const auto value : values) {
				const PRef<T> ref(value.second.release(), val.first);
				ret_vals.push_back(ref);
			}
			return std::move(ret_vals);
		}

		bool connect(std::string const& connection_string) {
			const auto ret = blib::bun::__private::DbBackend<blib::bun::__private::DbGenericType>::i().connect(connection_string);
			return ret;
		}
	}
}

/*
namespace soci{
	template<typename T>
	struct type_conversion<blib::bun::__private::SimpleObjHolder<T>>{
		using ObjectHolderType = blib::bun::__private::SimpleObjHolder<T>;
		typedef values base_type;

		struct FromBase{
		private:
			values const& val;

		public:
			template<typename T>
			void operator()(T& x) const {
				x = v.get<decltype(x)>("oid_low");
			}
		};

		static void from_base(values const& v, indicator ind, ObjectHolderType& obj) {
			indicator *i = &ind;
			T& o = *(obj->obj);
			obj.oid.low = v.get<decltype(o.oid.low)>("oid_low");
			obj.oid.high = v.get<decltype(o.oid.high)>("oid_high");
			boost::fusion::for_each(o, FromBase(v));
		}
	};
}
*/

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

#define GENERATE_TupType_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupType_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<blib::bun::__private::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePair_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<blib::bun::__private::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS_TUP)

#define EXPAND_member_names_I(z, n, ELEMS_TUP) ,BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

/// SPECIALIZE_BUN_HELPER Start
#define SPECIALIZE_BUN_HELPER(CLASS_ELEMS_TUP) BOOST_FUSION_ADAPT_STRUCT( BOOST_PP_TUPLE_REM_CTOR(CLASS_ELEMS_TUP) ) \
namespace blib{namespace bun{\
template<> struct IsPersistant<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> : boost::mpl::bool_<true> {};\
template<>\
struct CppTypeToDbType<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
static const DbTypes ret = DbTypes::kComposite;\
};\
}}\
namespace blib{namespace bun{namespace __private{\
template<> struct IsComposite<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> : boost::mpl::bool_<true> {};\
template<>\
struct TypeMetaData<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
using TupType = boost::fusion::vector<GENERATE_TupType(CLASS_ELEMS_TUP)>;\
using TupTypePairType = boost::fusion::vector<GENERATE_TupTypePair(CLASS_ELEMS_TUP)>;\
using T = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
inline static auto tuple_type_pair()->TupTypePairType const&{\
static const TupTypePairType t{GENERATE_TupTypePairObj(CLASS_ELEMS_TUP)};\
return t;\
}\
inline static std::string const& class_name(){\
static std::string const class_name = BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
return class_name;\
}\
inline static const std::vector<std::string>& member_names(){\
static const std::vector<std::string> names = {"oid_high", "oid_low" EXPAND_member_names(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))};\
return names;\
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
