#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file GlobalFunc.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Some helper function definitions for Object creation
///////////////////////////////////////////////////////////////////////////////

#include "blib/bun/PRefHelper.hpp"
#include "blib/bun/QueryHelper.hpp"
#include "blib/bun/DbBackend.hpp"

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
			blib::bun::__private::PRefHelper<T>::createSchema();
		}

		/// @fn deleteSchema
		/// @brief Delete the schema for the object
		template<typename T>
		inline static void deleteSchema() {
			blib::bun::__private::PRefHelper<T>::deleteSchema<T>();
		}

		template<typename T>
		inline static std::vector <SimpleOID> getAllOids() {
			return blib::bun::__private::QueryHelper<T>::getAllOids();
		}

		template<typename T>
		inline static std::vector <PRef<T>> getAllObjects() {
			return blib::bun::__private::QueryHelper<T>::getAllObjects();
		}

		template<typename T>
		inline static std::vector <PRef<T>> getAllObjWithQuery(std::string const &in_query) {
			return blib::bun::__private::QueryHelper<T>::getAllObjWithQuery(in_query);
		}

		bool connect(std::string const& connection_string) {
			const auto ret = blib::bun::__private::DbBackend<blib::bun::__private::DbGenericType>::i().connect(connection_string);
			return ret;
		}
	}
}