#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file NxNMappings.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Utilities and metafunctions for types using in doing NxN mappings
///////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <map>
#include "blib/bun/CppTypeToSQLString.hpp"

namespace blib {
	namespace bun {
		namespace __private {
			/////////////////////////////////////////////////
			/// @class TypeMetaData
			/// @brief Contains meta data that will be usieful to get type 
			///        inferences for a certain type.
			///////////////////////////////////////////////// 
			template<typename T>
			struct TypeMetaData {
				static std::string const& table_name();
				static std::map<std::string, std::string> const& type_maps();
			};

			template<typename T>
			struct NxNMappingTables {
				static std::string const& base_table_name();
				static std::string const& table_name();
			};

			template<typename T>
			struct OneToNCreationSql {
				static std::string const& sql() {
					return "";
				}
			};

			template<typename T>
			struct OneToNCreationSql<std::vector<T>> {
				static std::string const& table_name() {
					static const std::string table_name =
						NxNMappingTables<std::vector<T>>::base_table_name() +
						"_" +
						NxNMappingTables<std::vector<T>>::table_name();
					return table_name;
				}

				static std::string const& sql() {
					static const auto type_maps = TypeMetaData<T>::type_maps();
					static std::string sql_cols = "oid INTEGER PRIMARY KEY AUTOINCREMENT";

					for (auto it = type_maps.begin(); it != type_maps.end(); ++it) {
						sql_cols += "," + it->first + " " + it->second;
					}

					static const std::string sql = "CREATE TABLE IF NOT EXISTS '" +
						table_name() + "' (" + sql_cols + ")";
					return sql;
				}
			};
		}
	}
}