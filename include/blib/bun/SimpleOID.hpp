#pragma once

#include <array>
#include <cstdint>
#include <cstddef>
#include <chrono>

namespace blib {
  namespace bun {
    struct SimpleOID {
      typedef SimpleOID SelfType;
      typedef std::array<std::uint8_t, 16> OidByteArrayType;
      std::uint64_t high;
      std::uint64_t low;

      SimpleOID() = default;
      SimpleOID( const std::uint64_t in_high ) :
        high( in_high ),
        low( 0 ) {
        populateLow();
      }

      SimpleOID( const std::uint64_t in_high, const std::uint64_t in_low ) :
        high( in_high ),
        low( in_low ) {
      }

      void populateLow() {
        const auto t = std::chrono::high_resolution_clock::now();
        low = t.time_since_epoch().count();
      }

      void clear() {
        high = 0;
        low = 0;
      }

      SelfType& operator=( SelfType const& in_other ) {
        high = in_other.high;
        low = in_other.low;
        return *this;
      }

      bool operator==( SelfType const& in_other ) const {
        return high == in_other.high && low == in_other.low;
      }

      bool operator!=( SelfType const& in_other ) const {
        return !operator==( in_other );
      }

      OidByteArrayType toByteArray() const {
        union Convert {
          std::uint64_t var;
          std::uint8_t convert[8];
        };
        OidByteArrayType ret{ 0 };
        const Convert hc = { high };
        for (int i = 0; i < 8; ++i) {
          ret[i] = hc.convert[i];
        }

        const Convert lc = { low };
        for (int i = 0, j = 8; i < 8; ++i, ++j) {
          ret[j] = lc.convert[i];
        }

        return ret;
      }
    };
  }
}