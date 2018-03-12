#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file JSONUtils.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Some json utils.
///////////////////////////////////////////////////////////////////////////////

#include <string>

namespace blib {
	namespace bun {
		/// @fn tojson_string
		/// @brief Convert into a json representable key value
		template<typename T>
		auto tojson_string(T const& value)->T{
			return value;
		}

		auto tojson_string(std::string& value)->std::string {
			std::string& quote = std::string("'");
			const std::string ret = quote + value + quote;
			return std::move(ret);
		}
	}
}