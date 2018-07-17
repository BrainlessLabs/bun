#pragma once
#include<cstdint>
#include<boost/endian/conversion.hpp>

namespace bun {
	inline bool is_big_endian() {
		static const std::uint16_t val = 1;
		static const std::uint8_t* c = (std::uint8_t*)(&val);
		static const bool ret = *c ? false : true;
		return ret;
	}
}
