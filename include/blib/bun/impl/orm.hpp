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
#include <boost/fusion/include/algorithm.hpp>
#include <boost/fusion/include/filter_if.hpp>
#include <boost/fusion/include/copy.hpp>
#include <boost/preprocessor/tuple/rem.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/proto/proto.hpp>
#include <third_party/fmt/format.hpp>
#include <soci/error.h>
#include "blib/utils/MD5.hpp"
#include "blib/bun/impl/DbBackend.hpp"
#include "blib/bun/impl/DbLogger.hpp"
#include "blib/bun/impl/SimpleOID.hpp"
#include "blib/bun/impl/CppTypeToSQLString.hpp"

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
//#define EXPAND_CLASS_MEMBERS_createSchema_I(z, n, ELEMS_TUP) ", " BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP)) " {}"
//#define EXPAND_CLASS_MEMBERS_createSchema(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_CLASS_MEMBERS_createSchema_I, ELEMS_TUP)

namespace blib {
    namespace bun {
        template<typename T>
        struct PersistantNativeTypes {
            using ValueType = T;
            using DbFriendlyValueType = typename blib::bun::__private::ConvertCPPTypeToSOCISupportType<ValueType>::type;
            using SelfType = PersistantNativeTypes<ValueType>;

            DbFriendlyValueType val;

            PersistantNativeTypes() : val() {}
            PersistantNativeTypes(PersistantNativeTypes const& other) : val(other.val) {}
            PersistantNativeTypes(ValueType const other) : val(other) {}

            SelfType& operator=(SelfType const& other) {
                val = other.val;
            }

            SelfType& operator=(ValueType const& other) {
                val = other;
            }

            template<typename O>
            SelfType& operator=(O const& other) {
                val = blib::bun::__private::convertToSOCISupportedType<O>(other);
            }
        };
    }
}

namespace blib {
    namespace bun {
        /////////////////////////////////////////////////
        /// @class IsPersistant
        /// @brief If IsPersistant<Class>::value == true then Class can be persisted.
        ///        Specialize this class for all the class which can be persistent.
        /////////////////////////////////////////////////
        template<typename T>
        struct IsPersistant : boost::mpl::bool_<std::is_fundamental<T>::value || std::is_arithmetic<T>::value || std::is_floating_point<T>::value> {
        };

        template<>
        struct IsPersistant<std::string> : boost::mpl::bool_<true> {
        };

        namespace __private {
            /////////////////////////////////////////////////
            /// @fn to_valid_query_string
            /// @param val
            /// @brief Helper class to persist any primitive types.
            /////////////////////////////////////////////////
            template<typename T>
            inline auto to_valid_query_string(T const& val) -> T const& {
                T const& ret = val;
                return ret;
            }

            inline auto to_valid_query_string(std::string const& val, std::string const sym = "\"") -> std::string {
                const std::string ret_str = sym + val + sym;
                return ret_str;
            }

            inline auto to_valid_query_string(const char* val, std::string const sym = "\"") -> std::string {
                const std::string ret_str = sym + std::string(val) + sym;
                return ret_str;
            }

            /// @fn tojson_string
            /// @brief Convert into a json representable key value
            template<typename T>
            inline auto tojson_string(T const& value)->T {
                return value;
            }

            auto tojson_string(std::string& value)->std::string {
                const std::string quote("'");
                const std::string ret = quote + value + quote;
                return ret;
            }

            /// @fn to_json
            /// @brief Convert to json
            template<typename T>
            inline auto to_json(T const& value) -> std::string {
                //static_assert(blib::bun::IsPersistant<T>::value, "Cannot convert to json. Specialization for to_json conversion does not exist");
                const std::string ret = fmt::format("{}", value);
                return ret;
            }
        }
    }
}

namespace blib {
    namespace bun {
        namespace __private {
            template<typename T>
            struct ConfigurationOptions {
                inline static std::set<std::string>& unique_constraint_set() {
                    static std::set<std::string> constraint_set;
                    return constraint_set;
                }
            };

            /////////////////////////////////////////////////
            /// @class IsComposite
            /// @brief True if the element is a class/struct that can be persisted
            /////////////////////////////////////////////////
            template<typename T>
            struct IsComposite : boost::mpl::bool_<false> {
            };


            template< class T >
            struct remove_cvref {
                typedef std::remove_cv_t<std::remove_reference_t<T>> type;
            };

            /////////////////////////////////////////////////
            /// @class StripQualifiersAndMakePointer
            /// @brief String all the qualifiers like const, volatile, removes reference and adds a pointer.
            /////////////////////////////////////////////////
            template<typename T>
            struct StripQualifiersAndMakePointer {
                using type = typename std::add_pointer<typename remove_cvref<T>::type>::type;
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
            ///	@details Any Sql string has to be part of this class
            ///			 This class should provide a way for other classess to
            ///			 customize the string. So the strings constructed in this
            ///			 class is are flexible.
            /////////////////////////////////////////////////
            template<typename T>
            struct SqlString {
            private:
                struct SelectRows {
                private:
                    std::string& sql;

                public:
                    SelectRows(std::string& sql) : sql(sql) {}

                    template <typename O>
                    void operator()(O const& x) const
                    {
                        sql += "," + x.second;
                    }
                };

                struct CreateTable {
                private:
                    std::string& sql;

                public:
                    CreateTable(std::string& sql) : sql(sql) {}

                    template <typename O>
                    void operator()(O const& x) const
                    {
                        using ObjType = typename std::remove_const<typename std::remove_pointer<typename O::first_type>::type>::type;
                        static const std::string composite_type = blib::bun::cppTypeEnumToDbTypeString<DbTypes::kComposite>();
                        std::string type = blib::bun::cppTypeToDbTypeString<typename blib::bun::__private::ConvertCPPTypeToSOCISupportType<ObjType>::type>();
                        // If the type is of an object type then we will use VARCHAR to store the hash value
                        if (composite_type == type) {
                            type = "VARCHAR";
                        }
                        sql += "," + x.second + " " + type;
                    }
                };

                struct UpdateRow {
                private:
                    std::string& sql;

                public:
                    UpdateRow(std::string& sql) : sql(sql) {}

                    template <typename O>
                    void operator()(O const& x) const
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

                    template <typename O>
                    void operator()(O const& x) const
                    {
                        sql += ", " + x.second;
                    }
                };

                struct InsertRowVal {
                private:
                    std::string& sql;

                public:
                    InsertRowVal(std::string& sql) : sql(sql) {}

                    template <typename O>
                    void operator()(O const& x) const
                    {
                        sql += " , :" + x.second;
                    }
                };

            public:
                /// @fn create_table_sql
                /// @brief Create table sql
                inline static std::string const& create_table_sql() {
                    static const auto vecs = TypeMetaData<T>::tuple_type_pair();
                    static std::string sql;
                    if (sql.empty()) {
                        sql = "CREATE TABLE IF NOT EXISTS \"{}\" (oid VARCHAR PRIMARY KEY, oid_ref VARCHAR, parent_table_reference VARCHAR, parent_column_name VARCHAR";
                        boost::fusion::for_each(vecs, SqlString<T>::CreateTable(sql));
                        const auto& unique_constraint_set = ConfigurationOptions<T>::unique_constraint_set();
                        if (!unique_constraint_set.empty()) {
                            std::string unique_str = ", CONSTRAINT \"unique_constraint_" + TypeMetaData<T>::class_name() + "\" UNIQUE (";
                            bool include_comma = false;
                            for (const auto& elem : unique_constraint_set) {
                                if (include_comma) {
                                    unique_str += ",";
                                }
                                else {
                                    include_comma = true;
                                }
                                unique_str += elem;
                            }
                            unique_str += ")";
                            sql += unique_str;
                        }
                        sql += ")";
                    }
                    return sql;
                }

                /// @fn drop_table_sql
                /// @brief Drop table sql
                inline static std::string const& drop_table_sql() {
                    static const std::string sql = "DROP TABLE IF EXISTS \"{}\"";;
                    return sql;
                }

                /// @fn delete_row_condition_sql
                /// @brief Delete row sql with some condition
                /// @details This is useful for deleting child rows
                inline static std::string const& delete_row_condition_sql() {
                    static const std::string sql = "DELETE FROM \"{}\" WHERE {}";
                    return sql;
                }

                /// @fn delete_row_sql
                /// @brief Delete a row
                inline static std::string const& delete_row_sql() {
                    static const std::string sql = "DELETE FROM \"{}\" WHERE oid = :oid";
                    return sql;
                }

                /// @fn insert_row_sql
                /// @brief Insert row sql
                inline static std::string const& insert_row_sql() {
                    static const auto vecs = TypeMetaData<T>::tuple_type_pair();
                    static std::string sql;
                    if (sql.empty()) {
                        sql = "INSERT INTO \"{}\" (oid, oid_ref, parent_table_reference, parent_column_name";
                        boost::fusion::for_each(vecs, SqlString<T>::InsertRowNames(sql));
                        sql += ") VALUES ({}, {}, {}, {}";
                        boost::fusion::for_each(vecs, SqlString<T>::InsertRowVal(sql));
                        sql += ")";
                    }
                    return sql;
                }

                /// @fn update_row_sql
                /// @brief Update row sql
                inline static std::string const& update_row_sql() {
                    static const auto vecs = TypeMetaData<T>::tuple_type_pair();
                    static std::string sql;
                    if (sql.empty()) {
                        sql = "UPDATE \"{}\" SET oid_ref = {}, parent_table_reference = {}, parent_column_name = {},";
                        std::string sql1;
                        boost::fusion::for_each(vecs, SqlString<T>::UpdateRow(sql1));
                        sql += sql1 + " WHERE oid = {}";
                    }
                    return sql;
                }

                /// @fn select_rows_sql
                /// @brief Select row sql
                inline static std::string const& select_rows_sql() {
                    //static const auto vecs = TypeMetaData<T>::tuple_type_pair();
                    static const std::string sql = "SELECT * FROM \"{}\" ";
                    return sql;
                }

                /// @fn select_all_oid_sql
                /// @brief Select Oids sql
                inline static std::string const& select_all_oid_sql() {
                    static const std::string sql = "SELECT oid FROM \"{}\"";
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
                T* obj_ptr;
                blib::bun::SimpleOID const& oid;
                SimpleObjHolder(T* obj_ptr_in, blib::bun::SimpleOID const& oid_in) :obj_ptr(obj_ptr_in), oid(oid_in) {}
                ~SimpleObjHolder() {
                    obj_ptr = nullptr;
                }
            };

            /////////////////////////////////////////////////
            /// @class PRefHelper
            /// @brief Helper class for the persistent framework.
            ///        This class is specialized to persist objects.
            /////////////////////////////////////////////////
            template<typename T>
            struct QueryHelper {
                /// @fn createSchema
                /// @brief Create Schema
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

                /// @fn deleteSchema
                /// @brief Delete schema
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

                /// @fn persistObj
                /// @param obj This is the object that needs to be persisted
                /// @brief Persist an object
                inline static SimpleOID persistObj(T *obj, const SimpleOID& oid_ref = SimpleOID(),
                    std::string const& parent_table_reference = std::string(), std::string const& parent_column_name = std::string()) {
                    blib::bun::SimpleOID oid;
                    oid.populate();
                    const static std::string& class_name = TypeMetaData<T>::class_name();
                    const std::string sql = fmt::format(SqlString<T>::insert_row_sql(), class_name, to_valid_query_string(oid.to_string(), "'"),
                        to_valid_query_string(oid_ref.to_string(), "'"),
                        to_valid_query_string(parent_table_reference, std::string("'")), to_valid_query_string(parent_column_name, std::string("'")));
                    SimpleObjHolder<T> obj_holder(obj, oid);
                    QUERY_LOG(sql);
                    try {
                        blib::bun::__private::DbBackend<>::i().session() << sql, soci::use(obj_holder);
                    }
                    catch (std::exception const& e) {
                        l().error("persistObj(): {} ", e.what());
                    }
                    return oid;
                }

                /// @fn updateObj
                /// @param obj This is the object that needs to be updated
                /// @param oid the oid that needs to be updated
                /// @brief Persist an object.
                inline static void updateObj(T * obj, SimpleOID const& oid, const SimpleOID& oid_ref = SimpleOID(),
                    std::string const& parent_table_reference = std::string(), std::string const& parent_column_name = std::string()) {
                    static const std::string& class_name = TypeMetaData<T>::class_name();
                    const std::string sql = fmt::format(SqlString<T>::update_row_sql(),
                        class_name, to_valid_query_string(oid_ref.to_string(), "'"), to_valid_query_string(parent_table_reference, std::string("'")),
                        to_valid_query_string(parent_column_name, std::string("'")), to_valid_query_string(oid.to_string(), "'"));
                    SimpleObjHolder<T> obj_holder(obj, oid);
                    QUERY_LOG(sql);
                    try {
                        blib::bun::__private::DbBackend<>::i().session() << sql, soci::use(obj_holder);
                    }
                    catch (std::exception const& e) {
                        l().error("updateObj(): {} ", e.what());
                    }
                }

                /// @class DeleteObjects
                /// @brief Helper class to delete the objects of the enclosed scope too.
                struct DeleteObjects {
                private:
                    SimpleOID const& _oid;

                public:
                    DeleteObjects(SimpleOID const& oid) :_oid(oid) {}

                    template <typename O>
                    void operator()(O const& /*x*/) const {
                        QueryHelper<O>::deleteObjWithParentInfo(_oid);
                    }
                };

                /// @fn deleteObj
                /// @param oid The object associated with this oid needs to be deleted.
                /// @param obj The object that needs to be deleted
                /// @brief The object associated with this oid needs to be deleted.
                inline static void deleteObj(SimpleOID const& oid, T* obj = nullptr) {
                    static const std::string& class_name = TypeMetaData<T>::class_name();
                    const static std::string sql = fmt::format(SqlString<T>::delete_row_sql(), class_name);
                    QUERY_LOG(sql);
                    try {
                        if (obj != nullptr) {
                            // Delete all the nested objects.
                            boost::fusion::for_each(boost::fusion::filter_if<blib::bun::__private::IsComposite<boost::mpl::_>>(*obj), DeleteObjects(oid));
                        }

                        blib::bun::__private::DbBackend<>::i().session() << sql, soci::use(oid.to_string());
                    }
                    catch (std::exception const & e) {
                        l().error("deleteObj(): {} ", e.what());
                    }
                }

                /// @fn deleteObjWithParentInfo
                /// @param oid_ref
                /// @param parent_table_reference
                /// @param parent_column_name
                /// @brief The object associated with this oid needs to be deleted.
                inline static void deleteObjWithParentInfo(
                    const SimpleOID& oid_ref,
                    std::string const& parent_table_reference = std::string(),
                    std::string const& parent_column_name = std::string()) {
                    static const std::string& class_name = TypeMetaData<T>::class_name();
                    std::string where_clause = fmt::format("oid_ref = {}", to_valid_query_string(oid_ref.to_string(), "'"));

                    if (!parent_table_reference.empty()) {
                        where_clause += fmt::format(" AND parent_table_reference = {}", to_valid_query_string(parent_table_reference, "'"));
                    }

                    if (!parent_column_name.empty()) {
                        where_clause += fmt::format(" AND parent_column_name = {}", to_valid_query_string(parent_column_name, "'"));
                    }

                    const std::string sql = fmt::format(SqlString<T>::delete_row_condition_sql(), class_name, where_clause);
                    QUERY_LOG(sql);
                    try {
                        blib::bun::__private::DbBackend<>::i().session() << sql;
                    }
                    catch (std::exception const& e) {
                        l().error("deleteObjWithParentInfo({}): {} ", where_clause, e.what());
                    }
                }

                /// @fn getObj
                /// @param oid The oid for the object
                /// @brief The object with the particular oid will be returned.
                inline static std::unique_ptr<T> getObj(SimpleOID const & oid) {
                    static const std::string& class_name = TypeMetaData<T>::class_name();
                    const std::string sql = fmt::format(SqlString<T>::select_rows_sql() + " WHERE oid = {}",
                        class_name, to_valid_query_string(oid.to_string(), "'"));
                    QUERY_LOG(sql);
                    std::unique_ptr <T> obj = std::make_unique<T>();
                    SimpleObjHolder<T> obj_holder(obj.get(), oid);
                    try {
                        blib::bun::__private::DbBackend<>::i().session() << sql, soci::into(obj_holder);
                    }
                    catch (std::exception const& e) {
                        l().error("getObj(): {} ", e.what());
                    }
                    return std::move(obj);
                }

                /// @fn md5
                /// @param oid The oid for the object
                /// @brief Returns the md5 of the object
                inline static std::string md5(T const& obj, SimpleOID const & oid) {
                    const std::string json = QueryHelper<T>::objToJson(obj);
                    const static std::string& class_name = TypeMetaData<T>::class_name();
                    const std::string str = "{" + fmt::format("'oid': '{}', 'class_name': '{}', 'json': '{}'", oid.to_string(), class_name, json) + "}";
                    const std::string md5 = blib::md5(str);
                    return md5;
                }

                /// @fn objToString
                /// @param oid The oid for the object
                /// @param obj The object which we need to create to string
                /// @brief Converts the object to a string representation and
                ///		   returns the string representation.
                inline static std::string objToString(T const& obj) {
                    return QueryHelper<T>::objToJson(obj);
                }

                struct ToJson {
                private:
                    std::string& _str;
                    const std::vector<std::string>& _member_names;
                    int _count;

                public:
                    ToJson(std::string & str) :_str(str), _member_names(TypeMetaData<T>::member_names()), _count(2) {
                    }

                    template <typename O>
                    void operator()(O const& x) const {
                        const std::string member_name = _member_names.at(const_cast<ToJson*>(this)->_count++);
                        const std::string obj_name = blib::bun::__private::to_valid_query_string(member_name, "'");
                        _str += fmt::format("{} : {}", obj_name, to_json<O>(x));
                    }
                };

                /// @fn objToJson
                /// @param oid The oid for the object
                /// @param obj The object which we need to create to string
                /// @brief Converts the object to a json representation and
                ///		   returns the json representation as a string
                inline static std::string objToJson(T const& obj) {
                    std::string str;
                    boost::fusion::for_each(obj, QueryHelper<T>::ToJson(str));
                    str += "{" + str + "}";
                    return str;
                }


                template<typename TA, bool IsComposite>
                struct GetAllObjectsImpl {
                    inline static void impl(TA& x, const soci::row& row, const std::string& member_name, const std::string& /*oid_ref*/) {
                        x = row.get<typename ConvertCPPTypeToSOCISupportType<TA>::type>(member_name);
                    }
                };

                template<typename TA>
                struct GetAllObjectsImpl<TA, true> {
                    inline static void impl(TA& x, const soci::row& row, const std::string& member_name, const std::string& oid_ref) {
                        const std::string sql = fmt::format("oid_ref = {} AND parent_column_name = {}",
                            to_valid_query_string(oid_ref, "'"),
                            to_valid_query_string(member_name, "'"));
                        using ObjectRetType = std::pair<std::unique_ptr <TA>, SimpleOID>;
                        std::vector<ObjectRetType> objs = QueryHelper<TA>::getAllObjectsWithQuery(sql);
                        if (objs.empty()) {
                            l().error("GetAllObjectsImpl() for member name:{} has no elements ", member_name);
                        }
                        else {
                            ObjectRetType& obj = objs.at(0);
                            boost::fusion::copy(*obj.first, x);
                            //x;
                        }
                    }
                };

                struct GetAllObjects {
                private:
                    const soci::row& _row;
                    const std::string& _oid_ref;
                    const std::vector<std::string>& _member_names;
                    int _count;

                public:
                    GetAllObjects(const soci::row& row, const std::string& oid_ref) :_row(row), _oid_ref(oid_ref),
                        _member_names(TypeMetaData<T>::member_names()), _count(2) {
                    }

                    template <typename O>
                    void operator()(O& x) const {
                        const std::string& member_name = _member_names.at(const_cast<GetAllObjects*>(this)->_count++);
                        GetAllObjectsImpl<O, IsComposite<O>::value>::impl(x, _row, member_name, _oid_ref);
                        //x = _row.get<ConvertCPPTypeToSOCISupportType<O>::type>(_member_names.at(const_cast<GetAllObjects*>(this)->_count++));
                    }
                };

                /// @fn getAllObjectsWithQuery
                /// @param in_query Queries for which the objects will be returned.
                /// @brief The function will get all the objects that match the passed query
                /// @details https://www.citusdata.com/blog/2016/03/30/five-ways-to-paginate/
                ///			 Limit and Offset have its merits and demerits. Follow the page to learn them
                ///          We will be using Limit and Offset to start with the query building
                inline static std::vector<std::pair<std::unique_ptr <T>, SimpleOID>> getAllObjectsWithQuery(
                    const std::string& in_query = std::string(),
                    const std::size_t limit = 0,
                    const std::size_t offset = 0) {
                    std::vector<std::pair<std::unique_ptr <T>, SimpleOID>> ret_values;
                    const std::string& class_name = TypeMetaData<T>::class_name();
                    const static std::string select_sql = fmt::format(SqlString<T>::select_rows_sql(), class_name) + "{} {}";
                    const std::string where_clasue = in_query.empty() ? "" : "WHERE " + in_query + " ORDER BY 'oid'";
                    const std::string limit_clause = fmt::format(" LIMIT {} ", limit);
                    const std::string limit_offset_clause = limit ? limit_clause + fmt::format(" OFFSET {} ", offset) : "";
                    const std::string sql = fmt::format(select_sql, where_clasue, limit_offset_clause);
                    QUERY_LOG(sql);
                    try {
                        // Fetch the results as rowset
                        soci::rowset<soci::row> rows = (DbBackend<>::i().session().prepare << sql);
                        for (soci::rowset<soci::row>::const_iterator row_itr = rows.begin(); row_itr != rows.end(); ++row_itr) {
                            auto const& row = *row_itr;
                            std::pair<std::unique_ptr <T>, SimpleOID> pair;
                            pair.second = row.get<std::string>("oid");
                            pair.first = std::make_unique<T>();
                            T& obj = *pair.first;
                            boost::fusion::for_each(obj, QueryHelper<T>::GetAllObjects(row, pair.second.to_string()));
                            ret_values.emplace_back(pair.first.release(), pair.second);
                        }
                    }
                    catch (std::exception const & e) {
                        l().error("getAllObjectsWithQuery({}): {} ", in_query, e.what());
                    }
                    return std::move(ret_values);
                }


                /// @fn getAllNestedObjectssWithQuery
                /// @param in_query Queries for which the objects will be returned.
                /// @brief Get all the oids with that match the query.
                inline static std::vector<std::pair<std::unique_ptr <T>, SimpleOID>> getAllNestedObjectssWithQuery(std::string const in_query = std::string(),
                    const SimpleOID& oid_ref = SimpleOID(),
                    std::string const& parent_table_reference = std::string(), std::string const& parent_column_name = std::string()) {
                    const std::string query = fmt::format("{} AND oid_ref = {} AND parent_table_reference = {} AND parent_column_name = {}",
                        to_valid_query_string(oid_ref.to_string(), "'"),
                        to_valid_query_string(parent_table_reference, "'"),
                        to_valid_query_string(parent_column_name, "'"));
                    QUERY_LOG(query);
                    return QueryHelper<T>::getAllObjectsWithQuery(query);
                }

                /// @fn getAllOids
                /// @brief Get all the oids
                inline static std::vector<SimpleOID> getAllOids() {
                    const std::vector<SimpleOID> oids = QueryHelper<T>::getAllOidsWithQuery();
                    return oids;
                }

                /// @fn getAllOidsWithQuery
                /// @param in_query Queries for which the objects will be returned.
                /// @brief Get all the oids with that match the query.
                inline static std::vector<SimpleOID> getAllOidsWithQuery(
                    std::string const in_query = std::string(),
                    const std::size_t limit = 0,
                    const std::size_t offset = 0) {
                    std::vector<SimpleOID> oids;
                    const static std::string& class_name = TypeMetaData<T>::class_name();
                    const static std::string select_oid_sql = fmt::format(SqlString<T>::select_all_oid_sql(), class_name) + " {} ORDER BY oid {}";
                    const std::string where_clasue = in_query.empty() ? "" : "WHERE " + in_query;
                    const std::string limit_query = limit && offset ? "" : fmt::format("LIMIT {} OFFSET {}", limit, offset);
                    const std::string sql = fmt::format(select_oid_sql, where_clasue, limit_query);
                    QUERY_LOG(sql);
                    try {
                        const soci::rowset<soci::row> rows = (blib::bun::__private::DbBackend<>::i().session().prepare << sql);
                        for (auto row_itr = rows.begin(); row_itr != rows.end(); ++row_itr) {
                            auto const& row = *row_itr;
                            oids.emplace_back(row.get<std::string>("oid"));
                        }
                    }
                    catch (std::exception const & e) {
                        l().error("getAllOidsWithQuery({}): {} ", in_query, e.what());
                    }

                    return oids;
                }
            };
        }
    }
}

/// ======================PRef Class Start========================
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
            /// @typedef OidType = SimpleOID
            using OidType = SimpleOID;

            /// @var std::unique_ptr<T> _obj
            /// @brief Stores the object. The unique pointer is specialized
            ///        for that object type.
            std::unique_ptr<T> _obj;

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
            PRef() noexcept : _obj(), _md5(), oid() {}

            PRef(PRef const &in_other) : oid(in_other.oid) {
                load(oid);
            }

            PRef(PRef &in_other) :
                _obj(in_other._obj.release()),
                _md5(in_other._md5),
                oid(in_other.oid) {
            }

            PRef(ObjType *in_obj) : _obj(in_obj) {}

            PRef(OidType const &in_oid) : oid(in_oid) {
                load(oid);
            }

            PRef(OidType const &in_oid, ObjType *in_obj) : oid(in_oid), _obj(in_obj) {}

            /// @fn reset
            /// @brief Resets the current PRef and assigns another object to it.
            /// @param in_obj The other object to assign it to.
            void reset(ObjType *in_obj) {
                _obj.reset(in_obj);
                oid.clear();
                _md5.clear();
            }

            /// @fn release
            /// @brief Reset the PRef to hold nothing.
            ///        The oid is set to 0 and the object released.
            ObjType *release() {
                oid.clear();
                _md5.clear();
                return _obj.release();
            }

            ~PRef() {}

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
                const auto md5 = blib::bun::__private::QueryHelper<ObjType>::md5(*_obj.get(), oid);
                // If the md5 dont match then there is a change in the objects state.
                return md5 != _md5 ? true : false;
            }

            /// @fn persist
            /// @brief Commits the object in database
            /// @details Commits the object in database and also updates the
            ///          MD5 of the object. If this function is not called
            ///          the object is not going to be updated in database.
            /// @return OidType Returns the OID of the persisted object.
            OidType persist() {
                try {
                    if (_md5.empty()) {
                        oid = blib::bun::__private::QueryHelper<ObjType>::persistObj(_obj.get());
                    }
                    else {
                        blib::bun::__private::QueryHelper<ObjType>::updateObj(_obj.get(), oid);
                    }
                    _md5 = blib::bun::__private::QueryHelper<ObjType>::md5(*_obj.get(), oid);
                }
                catch (std::exception const & e) {
                    blib::bun::l().error(fmt::format("Exception while persisting: {}", e.what()));
                }
                catch (...) {
                    blib::bun::l().error(fmt::format("Unknown Exception while persisting: {}"));
                }

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
                blib::bun::__private::QueryHelper<ObjType>::deleteObj(oid, _obj.get());
                _md5.clear();
                oid.clear();
            }

            PRef& operator=(ObjType *in_obj) {
                reset(in_obj);
                return *this;
            }

            PRef& operator=(PRef& in_other) {
                copyFrom(in_other);
                return *this;
            }

            /*PRef& operator=(PRef const& in_other) {
                in_other.copyToOther(*this);
                return *this;
            }*/

            bool operator==(PRef const &in_other) const {
                return oid == in_other.oid;
            }

            bool operator!=(PRef const &in_other) const {
                return oid != in_other.oid;
            }

            /// @fn toJson
            /// @brief Returns a JSON representation of the object.
            std::string toJson() const {
                return blib::bun::__private::QueryHelper<T>::objToJson(*_obj.get());
            }

        private:
            void copyToOther(PRef& in_other) {
                in_other.oid = oid;
                in_other._obj.reset(const_cast<SelfType*>(this)->_obj.release());
                in_other._md5 = _md5;
            }

            /// @fn oad(OidType const &in_oid)
            /// @brief Loads an object from Database.
            /// @param in_oid A valid OID for the object.
            void load(OidType const &in_oid) {
                oid = in_oid;
                _obj = blib::bun::__private::QueryHelper<ObjType>::getObj(oid);
                _md5 = blib::bun::__private::QueryHelper<ObjType>::md5(*_obj.get(), oid);
            }

            /// @fn copyFrom(PRef &in_other)
            /// @brief Create a exact clone of another PRef
            void copyFrom(PRef& in_other) {
                oid = in_other.oid;
                _md5 = in_other._md5;
                _obj.reset(in_other._obj.release());
            }
        };
    }
}

/// ======================Global Functions Start========================
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
            blib::bun::__private::QueryHelper<T>::deleteSchema();
            t.commit();
        }

        /// @fn getAllOids
        /// @brief Get all the oids as a vector
        template<typename T>
        inline static std::vector <SimpleOID> getAllOids() {
            soci::transaction t(blib::bun::__private::DbBackend<>::i().session());
            const std::vector<SimpleOID> oids = blib::bun::__private::QueryHelper<T>::getAllOids();
            t.commit();
            return oids;
        }

        /// @fn getAllObjWithQuery
        /// @brief Get all the objects as PRef that satisfy the query
        template<typename T>
        inline static std::vector <PRef<T>> getAllObjWithQuery(
            std::string const &in_query,
            const std::size_t limit = 0,
            const std::size_t offset = 0) {
            soci::transaction t(blib::bun::__private::DbBackend<>::i().session());
            std::vector<std::pair<std::unique_ptr <T>, SimpleOID>> values = blib::bun::__private::QueryHelper<T>::getAllObjectsWithQuery(in_query, limit, offset);
            std::vector <PRef<T>> ret_vals;
            for (auto& value : values) {
                ret_vals.emplace_back(value.second, value.first.release());
            }
            t.commit();
            return std::move(ret_vals);
        }

        /// @fn getAllObjects
        /// @brief Get all the objects as PRef
        template<typename T>
        inline static std::vector <PRef<T>> getAllObjects() {
            //soci::transaction t(blib::bun::__private::DbBackend<>::i().session());
            return getAllObjWithQuery<T>(std::string());
            //t.commit();
        }

        /// @fn connect
        /// @brief Connect with the proper connection strings
        template<typename DbType = blib::bun::__private::DbGenericType>
        inline bool connect(std::string const& connection_string) {
            const auto ret = blib::bun::__private::DbBackend<blib::bun::__private::DbGenericType>::i().connect(connection_string);
            return ret;
        }

        /// @fn generateMetaData
        /// @brief Generate the metadata needed for various functionalities of Bun to work.
        inline bool generateMetaData() {
            return true;
        }
    }
}

/// ======================Fetch results and iterator helper Start========================
namespace blib {
    namespace bun {
        namespace __private {
            /// @class FetchResultsForQuery
            /// @brief This class fetches the data from the database.
            template<typename T>
            class FetchResultsForQuery {
            public:
                using ObjPRefVecType = decltype(blib::bun::getAllObjWithQuery<T>(""));
                    //using ObjPRefType = typename ObjPRefVecType::value_type;
                using ObjPRefType = ::blib::bun::PRef<T>;

            private:
                /// @var _query
                /// @brief The sql query generated
                std::string _query;
                /// @var _objects
                /// @brief The object cache. The objects at this current instance
                ObjPRefVecType _objects;
                /// @var _page_start
                /// @brief The pagination
                std::size_t _offset;
                /// @var _progress
                /// @brief How much the page should progress
                const std::size_t _limit;
                /// @var _cur_itr
                /// @brief The current iterator, that holds the current value
                typename ObjPRefVecType::iterator _cur_itr;
                /// @var _has_next
                /// @brief True if there is a next element and false if there is no element
                bool _has_next;

                static const std::size_t _initial_offset = 0;
                static const std::size_t _initial_limit = 1000;

            private:
                /// @fn populateObjectsFromDb
                /// @brief Fills the _objects with object from database. Resets the _offset to the next value and sets _has_next to true.
                void populateObjectsFromDb() {
                    _objects = blib::bun::getAllObjWithQuery<T>(_query, _limit, _offset);
                    if(!_objects.empty()) {
                        _cur_itr = _objects.begin();
                        _offset += _limit;
                        _has_next = true;
                    }
                }

                bool isEmpty() const {
                    bool ret = false;
                    if(_query.empty() || _objects.empty() ||
                       _offset == _initial_offset || _limit == _initial_limit ||
                       _cur_itr == _objects.end() || _has_next == false) {
                        ret = true;
                    }
                    return ret;
                }

                void progressNext() {
                    if (_objects.end() == _cur_itr) {
                        populateObjects();
                    }
                    else{
                        ++_cur_itr;
                    }

                    if(_objects.end() == _cur_itr) {
                        _has_next = false;
                    }
                }

                void populateObjects() {
                    populateObjectsFromDb();
                }

            public:
                /// @fn This is the default constructor. This does not set any values as such as the query is not set yet.
                FetchResultsForQuery() noexcept :
                    _offset(_initial_offset),
                    _limit(_initial_limit),
                    _cur_itr(_objects.end()),
                    _has_next(false){}

                FetchResultsForQuery(std::string& query):
                    _query(query),
                    _offset(_initial_offset),
                    _limit(_initial_limit),
                    _cur_itr(),
                    _has_next(false) {
                    populateObjects();
                }

                FetchResultsForQuery(FetchResultsForQuery& in_other) :
                    _query(in_other._query),
                    _objects(in_other._objects),
                    _offset(in_other._offset),
                    _limit(in_other._limit),
                    _cur_itr(_objects.begin()),
                    _has_next(in_other._has_next) {}

                FetchResultsForQuery& operator=(FetchResultsForQuery const& in_other) {
                    _objects = in_other._objects;
                    _offset = in_other._offset;
                    _limit = in_other._limit;
                    _cur_itr = _objects.begin();
                    _has_next = in_other._has_next;
                    return *this;
                }

                bool operator==(FetchResultsForQuery const& in_other) const {
                    bool is_same = false;
                    if(_objects == in_other._objects && _offset == in_other._offset &&
                    _limit == in_other._limit && _cur_itr == in_other._cur_itr && in_other._has_next == _has_next) {
                         is_same = true;
                    }
                    return is_same;
                }

                std::string const& query() const {
                    return _query;
                }

                bool ok() const{
                    return !isEmpty() || _cur_itr != _objects.end();
                }

                /// @fn reset
                /// @brief resets the query and all the objects fetched
                void reset(const std::string& query) {
                    _query = query;
                    _offset = _initial_offset;
                    _limit = _initial_limit;
                    _objects.clear();
                    _cur_itr = _objects.end();
                    _has_next = false;
                    populateObjects();
                }

                /// @fn hasNext
                /// @brief returns true if there are still objects in database
                /// else it returns false
                bool hasNext() const {
                    return _has_next;
                }

                /// @fn next
                /// @brief Returns the next element in the database
                /// @return The PRef value if the object is present or an empty PRef
                ObjPRefType& next() {
                    static ObjPRefType null_ref;
                    ObjPRefType& ret = hasNext() ? *_cur_itr : null_ref;
                    progressNext();
                    return ret;
                }

                /// @fn current
                /// @brief Returns the current object
                /// /// @return The PRef value if the object is present or an empty PRef
                ObjPRefType& current() const {
                    static ObjPRefType null_ref;
                    return ok() ? *_cur_itr : null_ref;;
                }

                /// @fn objects
                /// @brief Gets all the objects in the database. Should be used cautiously
                auto objects()->ObjPRefVecType& {
                    _objects = blib::bun::getAllObjWithQuery<T>(_query);
                    return _objects;
                }
            };
        }

        template<typename ValueType/*, typename PRefType=blib::bun::PRef<ValueType>*/>
        class ObjectIterator :
            public boost::iterator_facade<
            ObjectIterator<ValueType>,
            blib::bun::PRef<ValueType>/*PRefType*/,
            boost::forward_traversal_tag
            > {
        public:
            using SelfType = ObjectIterator<ValueType>;
            using FetchResultType = blib::bun::__private::FetchResultsForQuery<ValueType>;
            using PRef = blib::bun::PRef<ValueType>;

        private:
            friend class boost::iterator_core_access;
            FetchResultType _fetch_result;

        public:
            ObjectIterator() noexcept = default;
            ObjectIterator(std::string& query_string) :
                _fetch_result(query_string) {
            }

            ObjectIterator(ObjectIterator& in_other) :
                _fetch_result(in_other._fetch_result) {
            }

            bool operator!=(ObjectIterator const& in_other) const{
                return !equal(in_other);
            }

            bool operator==(ObjectIterator const& in_other) const{
                return equal(in_other);
            }

        private:
            void increment() {
                _fetch_result.next();
            }

            /// @todo Chack why the boost operator doesnt work only with this function in place.
            bool equal(ObjectIterator const& in_other) const {
                return _fetch_result.hasNext() == in_other._fetch_result.hasNext();
            }

            PRef& dereference() const {
                return _fetch_result.current();
            }
        };
    }
}
/// ======================Fetch results and iterator helper End========================

/// ======================Query Start========================
/// @brief The query templates starts here
/// @details This block of code refpresents the grammar and the structure of the
///			 query elements.
namespace blib {
    namespace bun {
        /// @class Configuration terminal
        struct UniqueConstraint {};
        namespace query {
            namespace __private {

                template<std::uint32_t I>
                struct QueryVariablePlaceholderIndex : std::integral_constant <std::uint32_t, I> {
                    QueryVariablePlaceholderIndex() noexcept {}
                };

                /// @brief Grammar for the query Start
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

                /// @class ConfigurationTerminals
                /// @brief Terminal for configuration
                struct ConfigurationTerminals : boost::proto::or_<
                    boost::proto::terminal<blib::bun::UniqueConstraint>
                > {};

                struct AllTerminals : boost::proto::or_<
                    PlaceHoldersTerminals,
                    QueryLiteralTerminals
                > {
                };

                /// @brief Configuration
                struct ConfigUniqueKeyGrammer : boost::proto::or_<
                    boost::proto::assign<AllTerminals, ConfigurationTerminals>
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
                    ConfigUniqueKeyGrammer,
                    boost::proto::logical_and<BunQueryGrammar, BunQueryGrammar>,
                    boost::proto::logical_or<BunQueryGrammar, BunQueryGrammar>
                > {
                };

                // Grammar for the query End

                /// @fn  mapping(const std::uint32_t in_index)
                /// @brief Gets the type element at position passed in by index
                /// {
                ///   static const std::vector<std::string> ret = { "name", "age", "height" };
                ///   return ret.at( in_index );
                /// }
                template<typename T>
                inline std::string const& mapping(const std::size_t in_index) {
                    static const auto vals = blib::bun::__private::TypeMetaData<T>::member_names();
                    return vals.at(in_index + 2); // member_names start from oid_high and oid_low
                }

                /// @class TypesUsed
                /// @brief boost::mpl::vector<decltype(test::Person::name), decltype(test::Person::age), decltype(test::Person::height)>;
                template<typename T>
                struct TypesUsed {
                    using Type = void;
                };

                /// @class FromInternals
                /// @brief Helper class to be used in From query
                template<typename T>
                struct FromInternals {
                    using TypesUsed = typename TypesUsed<T>::Type;

                    /// @class BunQueryFilterContex
                    /// @brief The context for filter queries in SQL
                    struct BunQueryFilterContex : boost::proto::callable_context<BunQueryFilterContex> {
                        typedef std::string result_type;

                        /// @fn BunQueryFilterContex
                        /// @brief default constructor
                        BunQueryFilterContex() noexcept {}

                        /// @fn operator()
                        /// @param The terminal tag
                        /// @param the terminal name
                        /// @brief returns valid terminal name
                        template<typename E>
                        result_type operator ()(boost::proto::tag::terminal, E in_term) const {
                            const auto ret = std::to_string(blib::bun::__private::to_valid_query_string(in_term));
                            return ret;
                        }

                        /// @fn operator()
                        /// @param The terminal tag
                        /// @param the terminal terminal name
                        /// @brief returns valid terminal name
                        result_type operator ()(boost::proto::tag::terminal, std::string& in_term) const {
                            const auto ret = blib::bun::__private::to_valid_query_string(in_term, std::string("'"));
                            return ret;
                        }

                        /// @fn operator()
                        /// @param The terminal tag
                        /// @param the terminal name
                        /// @brief returns valid terminal name. Overloaded for character
                        result_type operator ()(boost::proto::tag::terminal, char const* in_term) const {
                            std::string str = std::string(in_term);
                            const auto ret = blib::bun::__private::to_valid_query_string(str, std::string("'"));
                            return ret;
                        }

                        /// @fn operator()
                        /// @param The terminal tag
                        /// @param The index of the variable for lookup
                        /// @brief returns the name from the mapping
                        template<std::uint32_t I>
                        result_type operator()(boost::proto::tag::terminal, bun::query::__private::QueryVariablePlaceholderIndex<I> /*in_term*/) const {
                            const auto ret = mapping<T>(I);
                            return ret;
                        }

                        /// @fn operator()
                        /// @param Logical
                        /// @param left param
                        /// @param right param
                        /// @brief The query with the params
                        template<typename L, typename R>
                        result_type operator()(boost::proto::tag::logical_and, L const& in_l, R const& in_r) const {
                            static const std::string operator_name = " AND ";
                            auto ctx = *this;
                            const auto left_string = boost::proto::eval(in_l, ctx);
                            const auto right_string = boost::proto::eval(in_r, ctx);
                            const std::string ret = left_string + operator_name + right_string;
                            return ret;
                        }

                        template<typename L, typename R>
                        result_type operator()(boost::proto::tag::logical_or, L const& in_l, R const& in_r) const {
                            static const std::string operator_name = " OR ";
                            auto ctx = *this;
                            const auto left_string = boost::proto::eval(in_l, ctx);
                            const auto right_string = boost::proto::eval(in_r, ctx);
                            const std::string ret = left_string + operator_name + right_string;
                            return ret;
                        }

                        template<typename L, typename R>
                        result_type operator()(boost::proto::tag::less, L const& in_l, R const& in_r) const {
                            static const std::string operator_name = " < ";
                            auto ctx = *this;
                            const auto left_string = boost::proto::eval(in_l, ctx);
                            const auto right_string = boost::proto::eval(in_r, ctx);
                            const std::string ret = left_string + operator_name + right_string;
                            return ret;
                        }

                        template<typename L, typename R>
                        result_type operator()(boost::proto::tag::less_equal, L const& in_l, R const& in_r) const {
                            static const std::string operator_name = " <= ";
                            auto ctx = *this;
                            const auto left_string = boost::proto::eval(in_l, ctx);
                            const auto right_string = boost::proto::eval(in_r, ctx);
                            const std::string ret = left_string + operator_name + right_string;
                            return ret;
                        }

                        template<typename L, typename R>
                        result_type operator()(boost::proto::tag::greater, L const& in_l, R const& in_r) const {
                            static const std::string operator_name = " > ";
                            auto ctx = *this;
                            const auto left_string = boost::proto::eval(in_l, ctx);
                            const auto right_string = boost::proto::eval(in_r, ctx);
                            const std::string ret = left_string + operator_name + right_string;
                            return ret;
                        }

                        template<typename L, typename R>
                        result_type operator()(boost::proto::tag::greater_equal, L const& in_l, R const& in_r) const {
                            static const std::string operator_name = " >= ";
                            auto ctx = *this;
                            const auto left_string = boost::proto::eval(in_l, ctx);
                            const auto right_string = boost::proto::eval(in_r, ctx);
                            const std::string ret = left_string + operator_name + right_string;
                            return ret;
                        }

                        template<typename L, typename R>
                        result_type operator()(boost::proto::tag::equal_to, L const& in_l, R const& in_r) const {
                            static const std::string operator_name = " = ";
                            auto ctx = *this;
                            const auto left_string = boost::proto::eval(in_l, ctx);
                            const auto right_string = boost::proto::eval(in_r, ctx);
                            const std::string ret = left_string + operator_name + right_string;
                            return ret;
                        }

                        template<typename L, typename R>
                        result_type operator()(boost::proto::tag::not_equal_to, L const& in_l, R const& in_r) const {
                            static const std::string operator_name = " != ";
                            auto ctx = *this;
                            const auto left_string = boost::proto::eval(in_l, ctx);
                            const auto right_string = boost::proto::eval(in_r, ctx);
                            const std::string ret = left_string + operator_name + right_string;
                            return ret;
                        }

                        template<typename TerminalType, typename L, typename R>
                        result_type operator()(TerminalType, L const& /*in_l*/, R const& /*in_r*/) const {
                            //static_assert(false, "Operator not supported in Bun");
                            const std::string ret = "Operator not supported in Bun";
                            return ret;
                        }

                        template<typename TerminalType, typename L>
                        result_type operator()(TerminalType, L const& /*in_l*/) const {
                            //static_assert(false, "Operator not supported in Bun");
                            const std::string ret = "Operator not supported in Bun";
                            return ret;
                        }
                    };
                };
            }

            /// @brief Fields for query. This is a unknown namespace.
            namespace {
                /// @class F
                /// @brief This class represents the details of the query
                /// @details This class is specialized in the
                ///	         SPECIALIZE_BUN_HELPER for each specialized class.
                template<typename T>
                struct F;
            }

            /////////////////////////////////////////////////
            /// @brief From class for query.
            /////////////////////////////////////////////////
            template<typename T>
            struct From {
            public:
                    using ObjPRefVecType = typename blib::bun::__private::FetchResultsForQuery<T>::ObjPRefVecType;
                    using ObjPRefType = typename blib::bun::__private::FetchResultsForQuery<T>::ObjPRefType;
                    using ObjectIteratorType = blib::bun::ObjectIterator<T>;
            private:
                /// @var _query
                /// @brief The sql query generated
                std::string _query;
                blib::bun::__private::FetchResultsForQuery<T> _from_query;
                std::unique_ptr<ObjectIteratorType> _itr;

            private:
                template<typename ExpressionType>
                std::string eval(ExpressionType const& in_expr) {
                    typename __private::FromInternals<T>::BunQueryFilterContex ctx;
                    const std::string ret = boost::proto::eval(in_expr, ctx);
                    return ret;
                }

                static std::string const& className() {
                    static const auto table_name = blib::bun::__private::TypeMetaData<T>::class_name();
                    return table_name;
                }

                /// @fn append_query
                /// @param in_expr The query expression
                /// @param and_query if the value is true then the query is an and query, if and_query is false its an or query
                /// @brief Generates the query string
                template<typename ExpressionType>
                From& append_query(ExpressionType const& in_expr, const bool and_query = true) {
                    static_assert(boost::proto::matches<ExpressionType, __private::BunQueryGrammar>::value, "Syntax error in Bun Query");
                    const std::string query_string = eval(in_expr);
                    const std::string and_or_str = and_query ? " AND " : " OR ";
                    const std::string add_string = _query.empty() ? "" : and_or_str;
                    _query += add_string + query_string;
                    return *this;
                }

            public:
                From() noexcept :_from_query(_query), _itr(nullptr){}

                From(From& in_other) :_query(in_other._query), _from_query(in_other._query), _itr(nullptr){}

                template<typename ExpressionType>
                From& And(ExpressionType const& in_expr) {
                    return append_query(in_expr);
                }

                template<typename ExpressionType>
                From& Or(ExpressionType const& in_expr) {
                    return append_query(in_expr, false);
                }

                template<typename ExpressionType>
                From& where(ExpressionType const& in_expr) {
                    return append_query(in_expr);
                }

                template<typename ExpressionType>
                From& operator()(ExpressionType const& in_expr) {
                    return append_query(in_expr);
                }

                bool hasNext() {
                    return _from_query.hasNext();
                }

                auto next()->ObjPRefType {
                    return _from_query.next();
                }

                std::string const& query() const {
                    //l().info(_query);
                    return _query;
                }

                /// @fn objects
                /// @brief Gets all the objects
                auto objects()->ObjPRefVecType& {
                    return _from_query.objects();
                }

                ObjectIteratorType& begin() {
                    _itr = std::make_unique<ObjectIteratorType>(_query);
                    return *_itr;
                }

                ObjectIteratorType& end() const {
                    static std::unique_ptr<ObjectIteratorType> end_itr = std::make_unique<ObjectIteratorType>();
                    return *end_itr;
                }
            };

            template<typename ExpressionType>
            struct IsValidQuery : std::integral_constant<bool, boost::proto::matches<ExpressionType, __private::BunQueryGrammar>::value> {
            };
        }
    }
}
/// ======================Query End========================

/// ======================Configuration Start========================
namespace blib {
    namespace bun {
        namespace __private {
            /// @class UniqueConstraint
            /// @brief Helper class to be used to add
            template<typename T>
            struct UniqueConstraintInternal {
                //using TypesUsed = typename TypesUsed<T>::Type;
                /// @class BunConstraintContex
                /// @brief The context for Unique Constraint
                struct BunConstraintContex : boost::proto::callable_context<BunConstraintContex> {
                    using result_type = std::string;

                    /// @fn BunConstraintContex
                    /// @brief default constructor
                    BunConstraintContex() {}

                    /// @fn operator()
                    /// @param The terminal tag
                    /// @param the terminal name
                    /// @brief returns valid terminal name
                    /*template<typename E>
                    result_type operator ()(boost::proto::tag::terminal, E in_term) const {
                    const auto ret = std::to_string(blib::bun::__private::to_valid_query_string(in_term));
                    return ret;
                    }*/

                    /// @fn operator()
                    /// @param The terminal tag
                    /// @param the terminal terminal name
                    /// @brief returns valid terminal name
                    result_type operator ()(boost::proto::tag::terminal, std::string& in_term) const {
                        const auto ret = blib::bun::__private::to_valid_query_string(in_term, std::string("'"));
                        return ret;
                    }

                    /// @fn operator()
                    /// @param The terminal tag
                    /// @param the terminal name
                    /// @brief returns valid terminal name. Overloaded for character
                    result_type operator ()(boost::proto::tag::terminal, char const* in_term) const {
                        std::string str = std::string(in_term);
                        const auto ret = blib::bun::__private::to_valid_query_string(str, std::string("'"));
                        return ret;
                    }
                    /// @fn operator()
                    /// @param The terminal tag
                    /// @param The index of the variable for lookup
                    /// @brief returns the name from the mapping
                    template<std::uint32_t I>
                    result_type operator()(boost::proto::tag::terminal, ::blib::bun::query::__private::QueryVariablePlaceholderIndex<I> /*in_term*/) const {
                        const result_type ret = ::blib::bun::query::__private::mapping<T>(I);
                        return ret;
                    }

                    result_type operator()(boost::proto::tag::terminal, blib::bun::UniqueConstraint& /*in_term*/) const {
                        static const result_type ret = "UNIQUE";
                        return ret;
                    }

                    template<typename L, typename R>
                    result_type operator()(boost::proto::tag::assign, L const& in_l, R const& /*in_r*/) const {
                        auto ctx = *this;
                        const auto left_string = boost::proto::eval(in_l, ctx);
                        //const auto right_string = boost::proto::eval(in_r, ctx);
                        ConfigurationOptions<T>::unique_constraint_set().insert(left_string);
                        const result_type ret = left_string;
                        return ret;
                    }
                };
            };
        } // __private

          /////////////////////////////////////////////////
          /// @brief Cinfiguration class.
          /////////////////////////////////////////////////
        template<typename T>
        struct Configuration {
        private:
            template<typename ExpressionType>
            std::string eval(ExpressionType const& in_expr) {
                typename __private::UniqueConstraintInternal<T>::BunConstraintContex ctx;
                const std::string ret = boost::proto::eval(in_expr, ctx);
                return ret;
            }

            static std::string const& className() {
                static const auto table_name = blib::bun::__private::TypeMetaData<T>::class_name();
                return table_name;
            }

        public:
            Configuration() = default;

            template<typename ExpressionType>
            Configuration& set(ExpressionType const& in_expr) {
                static_assert(boost::proto::matches<ExpressionType, blib::bun::query::__private::BunQueryGrammar>::value, "Syntax error in Bun Configuration Expression");
                const std::string query_string = eval(in_expr);
                return *this;
            }

            template<typename ExpressionType>
            Configuration& operator()(ExpressionType const& in_expr) {
                return set(in_expr);
            }

            std::set<std::string> const& uniques() const {
                return bun::__private::ConfigurationOptions<T>::unique_constraint_set();
            }
        };

        namespace {
            static blib::bun::UniqueConstraint unique_constraint;
        }
    }
}
/// ======================Configuration End========================

/// @brief Transaction
namespace blib {
    namespace bun {
        /// @class Transaction
        /// @brief The class that handles transaction
        /// @details The transaction class is a wrapper around the transaction
        ///			of SOCI. This supports all the methods of the soci::transaction class.
        class Transaction {
        private:
            soci::transaction _t;

        public:
            Transaction() noexcept :_t(blib::bun::__private::DbBackend<>::i().session()) {
            }

            ~Transaction() {
                _t.~transaction();
            }

            void commit() {
                _t.commit();
            }

            void rollback() {
                _t.rollback();
            }
        };
    }
}

/// @brief SOCI conversion helpers
namespace blib {
    namespace bun {
        namespace __private {
            /// @class type_conversion
            /// @brief partial specialization to support regular objects T
            /// @details
            template<typename T>
            struct type_conversion {
            public:
                using ObjType = T;
            private:
                struct FromBase {
                private:
                    soci::values const& _val;
                    std::uint_fast32_t _count;

                public:
                    FromBase(soci::values const& val) :_val(val), _count(2) {}

                    template<typename O>
                    void operator()(O& x) const {
                        const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<FromBase*>(this)->_count++);
                        x = _val.get<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name);
                    }
                };

            public:
                inline static void from_base(soci::values const& v, soci::indicator, ObjType& obj) {
                    //boost::fusion::for_each(obj, FromBase(v));
                }

            private:
                struct ToBase {
                private:
                    soci::values& _val;
                    std::uint_fast32_t _count;

                public:
                    ToBase(soci::values& val) :_val(val), _count(2) {}

                    template<typename O>
                    void operator()(O const& x) const {
                        const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<ToBase*>(this)->_count++);
                        const_cast<ToBase*>(this)->_val.set(obj_name, x);
                    }
                };

            public:
                inline static void to_base(ObjType const& obj, soci::values& v, soci::indicator& ind) {
                    //boost::fusion::for_each(obj, ToBase(v));
                }
            };


            /// @class type_conversion
            /// @brief partial specialization to support SimpleObjHolder<T>
            /// @details
            template<typename T>
            struct type_conversion<SimpleObjHolder<T>> {
                using ObjectHolderType = SimpleObjHolder<T>;
                using ObjType = T;

            private:
                template<typename O, bool IsComposite = false>
                struct FromBaseOperation {
                    inline static void execute(O& x, const std::string& obj_name, soci::values const& val, const blib::bun::SimpleOID& /*parent_oid*/) {
                        x = val.get<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name);
                    }
                };

                template<typename O>
                struct FromBaseOperation<O, true> {
                    inline static void execute(O& x, const std::string& obj_name, soci::values const& val, const blib::bun::SimpleOID& parent_oid) {
                        const blib::bun::SimpleOID& oid_ref = parent_oid;
                        const std::string& parent_table_reference = TypeMetaData<ObjType>::class_name();
                        // TODO
                        using RetType = std::vector<std::pair<std::unique_ptr <O>, SimpleOID>>;
                        const RetType values = blib::bun::__private::QueryHelper<O>::getAllNestedObjectssWithQuery("", oid_ref, parent_table_reference, obj_name);
                        if (!values.empty()) {
                            x = *values.at(0).first;
                        }
                        else {
                            l().error("FromBaseOperation::execute: Could not fetch object values");
                        }
                    }
                };

                struct FromBase {
                private:
                    soci::values const& _val;
                    const blib::bun::SimpleOID& _oid;
                    std::uint16_t _count;
                    soci::indicator& _ind;

                public:
                    FromBase(soci::values const& val, blib::bun::SimpleOID const& oid, soci::indicator& ind) :_val(val), _oid(oid), _count(2), _ind(ind) {}

                    template<typename O>
                    void operator()(O& x) const {
                        const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<FromBase*>(this)->_count++);
                        FromBaseOperation<O, IsComposite<O>::value>::execute(x, obj_name, _val, _oid);
                    }
                };

            public:
                /// @fn from_base Setting the values in the object
                /// @brief This will take the database value and put it in the object. Database -> Object
                inline static void from_base(soci::values const& v, soci::indicator ind, ObjectHolderType& obj_holder) {
                    ObjType& obj = *(obj_holder.obj_ptr);
                    const blib::bun::SimpleOID& oid = obj_holder.oid;
                    boost::fusion::for_each(obj, FromBase(v, oid, ind));
                }

            private:
                template<typename O, bool IsComposite = false>
                struct ToBaseOperation {
                    inline static void execute(O& x, const std::string& obj_name, soci::values& val, const blib::bun::SimpleOID& parent_oid, soci::indicator& ind) {
                        val.set<typename ConvertCPPTypeToSOCISupportType<typename std::remove_reference<O>::type>::type>(obj_name, x, ind);
                    }
                };

                template<typename O>
                struct ToBaseOperation<O, true> {
                    inline static void execute(O& x, const std::string& obj_name, soci::values& val, const blib::bun::SimpleOID& parent_oid, soci::indicator& ind) {
                        const blib::bun::SimpleOID& oid_ref = parent_oid;
                        const std::string& parent_table_reference = TypeMetaData<ObjType>::class_name();
                        const std::string& parent_column_name = obj_name;
                        // Do not delete if parent oid is nill. If parent
                        // oid is nill then this object is not yet persisted.
                        if (oid_ref.empty() != 0) {
                            // Delete everything before inserting. There is no
                            // update essentially. Just delete and insert of any nested objects.
                            QueryHelper<O>::deleteObj(oid_ref, &x);
                        }
                        // Get the oid of the nested object after persisting it
                        const blib::bun::SimpleOID oid = QueryHelper<O>::persistObj(&x, oid_ref, parent_table_reference, parent_column_name);
                        // Get the json representation of the nested object and store it in the parent object
                        const std::string oids = oid.to_json();
                        // TODO : Fix this. This has the context of current query execution. Needs to set the value in parent.
                        val.set<typename ConvertCPPTypeToSOCISupportType<std::string>::type>(obj_name, oids, ind);
                    }
                };

                struct ToBase {
                private:
                    soci::values& _val;
                    const blib::bun::SimpleOID& _oid;
                    std::uint_fast32_t _count;
                    soci::indicator& _ind;

                public:
                    ToBase(soci::values& val, blib::bun::SimpleOID const& oid, soci::indicator& ind) :_val(val), _oid(oid), _count(2), _ind(ind) {}

                    template<typename O>
                    void operator()(O& x) const {
                        const std::string obj_name = TypeMetaData<ObjType>::member_names().at(const_cast<ToBase*>(this)->_count++);
                        ToBaseOperation<O, IsComposite<O>::value>::execute(x, obj_name, _val, _oid, _ind);
                    }
                };

            public:
                /// @fn to_base Setting the values in the database
                /// @brief This will take the object value and persist it in the database. Object -> Database
                inline static void to_base(ObjectHolderType& obj_holder, soci::values& v, soci::indicator& ind) {
                    ObjType& obj = *(obj_holder.obj_ptr);
                    const blib::bun::SimpleOID& oid = obj_holder.oid;
                    boost::fusion::for_each(obj, ToBase(v, oid, ind));
                }
            };
        }
    }
}

namespace soci {
    /// @class type_conversion
    /// @brief SOCI class specialized to use SimpleObjHolder
    template<typename T>
    struct type_conversion<blib::bun::__private::SimpleObjHolder<T>> {
        using ObjectHolderType = blib::bun::__private::SimpleObjHolder<T>;
        using ObjType = T;
        typedef values base_type;

        /// @fn from_base
        /// @param soci::values const& v
        /// @param soci::indicator ind
        /// @param ObjectHolderType& obj_holder
        inline static void from_base(soci::values const& v, soci::indicator ind, ObjectHolderType& obj_holder) {
            blib::bun::__private::type_conversion<ObjectHolderType>::from_base(v, ind, obj_holder);
        }

        /// @fn to_base
        /// @param ObjectHolderType const& obj_holder
        /// @param soci::values& v
        /// @param soci::indicator& ind
        inline static void to_base(ObjectHolderType& obj_holder, soci::values& v, soci::indicator& ind) {
            const std::string oid = obj_holder.oid.to_string();
            blib::bun::__private::type_conversion<ObjectHolderType>::to_base(obj_holder, v, ind);
        }
    };
}

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros Start
///////////////////////////////////////////////////////////////////////////////
#define GENERATE_TupType_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() std::add_pointer<std::remove_reference<std::remove_cv<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>::type>::type
#define GENERATE_TupType(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupType_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePair_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::pair<blib::bun::__private::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type, std::string>
#define GENERATE_TupTypePair(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePair_I, CLASS_ELEMS_TUP)

#define GENERATE_TupTypePairObj_I(z, n, CLASS_ELEMS_TUP) BOOST_PP_IF(n, BOOST_PP_COMMA, BOOST_PP_EMPTY)() boost::fusion::make_pair<blib::bun::__private::StripQualifiersAndMakePointer<decltype(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) :: BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP))>::type>(BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)))
#define GENERATE_TupTypePairObj(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), GENERATE_TupTypePairObj_I, CLASS_ELEMS_TUP)

#define EXPAND_member_names_I(z, n, ELEMS_TUP) ,BOOST_STRINGIZE(BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP))
#define EXPAND_member_names(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), EXPAND_member_names_I, ELEMS_TUP)

/// @brief generate the query and query fields
#define DEFINE_CLASS_STATIC_VARS_QUERY_I(z, n, CLASS_ELEMS_TUP) boost::proto::terminal<blib::bun::query::__private::QueryVariablePlaceholderIndex<n>>::type const F<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(n, 1), CLASS_ELEMS_TUP);
#define DEFINE_CLASS_STATIC_VARS_QUERY(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), DEFINE_CLASS_STATIC_VARS_QUERY_I, CLASS_ELEMS_TUP)

#define GENERATE_CLASS_STATIC_VARS_QUERY_I(z, n, ELEMS_TUP) static boost::proto::terminal<blib::bun::query::__private::QueryVariablePlaceholderIndex<n>>::type const BOOST_PP_TUPLE_ELEM(n, ELEMS_TUP);
#define GENERATE_CLASS_STATIC_VARS_QUERY(ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_TUPLE_SIZE(ELEMS_TUP), GENERATE_CLASS_STATIC_VARS_QUERY_I, ELEMS_TUP)

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
template<>\
inline auto to_json<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP) const& value) -> std::string {\
return QueryHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>::objToJson(value);\
}\
}}}\
namespace blib{ namespace bun{ namespace query{\
namespace {\
template<>\
struct F<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> {\
GENERATE_CLASS_STATIC_VARS_QUERY(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP ))\
};\
DEFINE_CLASS_STATIC_VARS_QUERY(CLASS_ELEMS_TUP)\
}\
}}}\

/// SPECIALIZE_BUN_HELPER End
