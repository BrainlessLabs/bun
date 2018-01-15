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
#include <third_party/fmt/format.hpp>
#include "blib/bun/PRef.hpp"
//#include "blib/bun/PRefHelper.hpp"
#include "blib/bun/DbBackend.hpp"
#include "blib/bun/DbLogger.hpp"
//#include "blib/bun/QueryHelper.hpp"
#include "blib/bun/CppTypeToSQLString.hpp"
#include "blib/bun/GlobalFunc.hpp"
#include "blib/utils/JSONUtils.hpp"
#include "blib/utils/TypeUtils.hpp"
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

namespace blib {
	namespace bun {
		namespace __private {
			/////////////////////////////////////////////////
			/// @class PRefHelper
			/// @brief Helper class for the persistent framework.
			///        This class is specialized to persist objects.
			/////////////////////////////////////////////////
			template<typename T>
			struct QueryHelper {
				static std::string table_name;
				static std::map<std::string, TypeDetails> type_details;

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
