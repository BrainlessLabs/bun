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
#include <type_traits>
#include <boost/preprocessor.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/include/sequence.hpp>
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
    struct IsPersistant : std::integral_constant<bool, std::is_arithmetic<T>::value> {
    };

    template<>
    struct IsPersistant<std::string> : std::true_type {
    };
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
			struct FindEncloseeTypeMeta{
				using type = T;
			};
			
			template<typename T>
			struct FindEncloseeTypeMeta<std::vector<T>>{
				using type = T;
			};
			
			template<typename T>
			struct FindEncloseeTypeMeta<std::unique_ptr<T>>{
				using type = T;
			};
			
			template<typename T>
			struct FindEncloseeTypeMeta<std::shared_ptr<T>>{
				using type = T;
			};					
			
			/////////////////////////////////////////////////
			/// @class FindEnclosureTypeMeta
			/// @brief Deducts the enclosing type
			/////////////////////////////////////////////////			
			template<typename T>
			struct FindEnclosureTypeMeta{
				static const EnclosureType = EnclosureType::kBaseType;
			};
			
			template<typename T>
			struct FindEnclosureTypeMeta<std::vector<T>>{
				static const EnclosureType = EnclosureType::kVector;
			};
			
			template<typename T>
			struct FindEnclosureTypeMeta<std::unique_ptr<T>>{
				static const EnclosureType = EnclosureType::kUniquePointer;
			};
			
			template<typename T>
			struct FindEnclosureTypeMeta<std::shared_ptr<T>>{
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
				bool in_fundamental_type):type(in_type),
				enclouser_type(in_enclouser_type),
				fundamental_type(in_fundamental_type){}
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
				static std::map<std::string, TypeDetails> const& type_maps();
			};
			
			template<typename T>
			struct SqlHelper{
				static std::string const& createSchema(){
					static const std::string sql = _createSchema();
					return sql;
				}
				
			private:
				static std::string _createSchema(){
					std::string sql;
					return sql;
				}
			};
			
			/////////////////////////////////////////////////
			/// @class PRefHelper
			/// @brief Helper class for the persistent framework.
			///        This class is specialized to persist objects.
			/////////////////////////////////////////////////
			template<typename T>
			struct QueryHelper {
				static std::string table_name;

				inline static void createSchema();

				inline static void createSchema(const std::string parent_table, const std::string table);

				inline static void deleteSchema();

				inline static SimpleOID persistObj(T *);

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
blib::bun::CppTypeToDbType<FindEncloseeTypeMeta<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n)>::type>::ret,\
FindEnclosureTypeMeta<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n), CLASS_ELEMS_TUP)>::type,\
std::is_fundamental<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)::BOOST_PP_TUPLE_ELEM(BOOST_PP_ADD(1, n)>::value\
)\
},
#define EXPAND_MEMBER_ASSIGNENTS_generate_type_maps(CLASS_ELEMS_TUP) BOOST_PP_REPEAT(BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(CLASS_ELEMS_TUP), 1), EXPAND_MEMBER_ASSIGNENTS_generate_type_maps_I, CLASS_ELEMS_TUP)

///////////////////////////////////////////////////////////////////////////////
/// Helper Macros End
///////////////////////////////////////////////////////////////////////////////

/// SPECIALIZE_BUN_HELPER Start
#define SPECIALIZE_BUN_HELPER(CLASS_ELEMS_TUP) namespace blib{namespace bun{\
template<>\
BLIB_MACRO_COMMENTS_IF("@brief Lets everyone know that this is a persistant class");\
struct IsPersistant<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)> : std::true_type {\
};\
BLIB_MACRO_COMMENTS_IF("@brief Mark the class as composite");\
template<>\
struct CppTypeToDbType<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
static const DbTypes ret = DbTypes::kComposite;\
};\
}\
namespace blib{namespace bun{namespace __private{\
template<>\
TypeMetaData<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
using T=BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
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
template<>\
struct QueryHelper<BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP)>{\
using T = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
inline static void createSchema(){\
BLIB_MACRO_COMMENTS_IF("@brief createSchema for creating the schema of an object");\
static std::string const class_name = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP));\
static std::string const query = "CREATE TABLE IF NOT EXISTS '{}' (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER NOT NULL" \
EXPAND_CLASS_MEMBERS_createSchema(BOOST_PP_TUPLE_POP_FRONT( CLASS_ELEMS_TUP )) \
 ")";\
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
