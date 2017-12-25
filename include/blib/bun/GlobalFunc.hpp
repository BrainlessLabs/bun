#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file GlobalFunc.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Some helper function definitions for Object creation
///////////////////////////////////////////////////////////////////////////////

#include "blib/bun/PRefHelper.hpp"

namespace blib {
	namespace bun {
		/////////////////////////////////////////////////
		/// @class GlobalFunc
		/// @brief Helper class for the persistent framework.
		///        This class is specialized to persist objects.
		/////////////////////////////////////////////////
		template<typename T>
		inline static void createSchema() {
			blib::bun::__private::PRefHelper<T>::createSchema();
		}

		template<typename T>
		inline static void deleteSchema() {
			blib::bun::__private::PRefHelper<T>::deleteSchema<T>();
		}
	}
}