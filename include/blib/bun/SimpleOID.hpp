#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file SimpleOID.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Class for generating and holding an OID
///////////////////////////////////////////////////////////////////////////////

#include <array>
#include <cstdint>
#include <cstddef>
#include <chrono>
#include <third_party/fmt/format.hpp>

namespace blib {
    namespace bun {
        /// @class SimpleOID
        /// @brief Class to generate and hold a simple OID.
        /// @details The OID generation is very simple.
        ///          The low value can be generate using the current time.
        ///          High should be filled to create a unique OID.
        ///          Both high and low are of std::uint64_t
        struct SimpleOID {
            /// @typedef SelfType=SimpleOID
            using SelfType=SimpleOID;
            /// @typedef OidByteArrayType=std::array<std::uint8_t, 16>
            /// @brief To hold the 16 byte array representation of the OID.
            using OidByteArrayType=std::array<std::uint8_t, 16>;
			/// @typedef OidHighType = std::uint64_t
			/// @brief Type to hold high value
			using OidHighType = std::uint64_t;
			/// @typedef OidLowType = std::uint64_t
			/// @brief Type to hold low value
			using OidLowType = std::uint64_t;
            /// @var std::uint64_t high
            /// @brief hold the higher order bytes.
			OidHighType high;
            /// @var std::uint64_t low
            /// @brief holds the lower order bytes.
			OidLowType low;

			SimpleOID() : high(0), low(0) {}

            ~SimpleOID() = default;

            /// @fn SimpleOID
            /// @brief Populates the low automatically. high should be passed
            /// @param in_high Pass the param to put it into high
            SimpleOID(const OidHighType in_high) :
                    high(in_high),
                    low(0) {
                populateLow();
            }

            /// @fn SimpleOID
            /// @brief Pass both the high and low to be populated.
            /// @param in_high Passed to high
            /// @param in_low Passed to low
            SimpleOID(const OidHighType in_high, const OidLowType in_low) :
                    high(in_high),
                    low(in_low) {
            }

            /// @fn populateLow
            /// @brief Get the current time and populate low.
            ///        Uses the std::crono for getting the time.
            void populateLow() {
                const auto t = std::chrono::high_resolution_clock::now();
                low = static_cast<OidLowType>(t.time_since_epoch().count());
            }

            /// @fn populateAll
            /// @brief Get the current time and populate high and low both.
            ///        Uses the std::crono for getting the time.
            void populateAll() {
                const auto t = std::chrono::high_resolution_clock::now();
                low = static_cast<OidHighType>(t.time_since_epoch().count());
                high = static_cast<OidLowType>(t.time_since_epoch().count());
            }

            /// @fn clear
            /// @brief clears the high and low values.
            void clear() {
                high = 0;
                low = 0;
            }

            /// @fn operator=
            /// @brief Copies the value of high and low from the RHS. Deep copy
            SelfType &operator=(SelfType const &in_other) {
                high = in_other.high;
                low = in_other.low;
                return *this;
            }

            /// @fn operator==
            /// @brief Compares high and low of the LHS and RHS.
            ///        Returns true when both low and high are same.
            /// @return true when both low and high are same else false
            bool operator==(SelfType const &in_other) const {
                return high == in_other.high && low == in_other.low;
            }

            /// @fn operator!=
            /// @brief Returns true if either or both of low and high
            ///        does not match. Internally calls operator==
            /// @return false only when both high and low matches else true
            bool operator!=(SelfType const &in_other) const {
                return !operator==(in_other);
            }

            /// @fn toByteArray
            /// @brief Convert the OID to byte array and then return.
            ///        The byte representation is not stored, so every time
            ///        this function is called this will generate again.
            /// @return OidByteArrayType 16 Byte representation of the OID
            OidByteArrayType toByteArray() const {
                /// @class ConvertHigh
                /// @brief Union to convert a uint64 into 8 uint8 bytes
                union ConvertHigh {
					OidHighType var;
                    std::uint8_t convert[8];
                };
                OidByteArrayType ret{0};
                // Convert the high value
                const ConvertHigh hc = {high};
                for (int i = 0; i < 8; ++i) {
                    ret[i] = hc.convert[i];
                }

				/// @class ConvertHigh
				/// @brief Union to convert a uint64 into 8 uint8 bytes
				union ConvertLow {
					OidLowType var;
					std::uint8_t convert[8];
				};
                // Convert the low value and append
                const ConvertLow lc = {low};
                for (int i = 0, j = 8; i < 8; ++i, ++j) {
                    ret[j] = lc.convert[i];
                }

                return ret;
            }
            
            /// @fn to_json
            std::string to_json() const {
                const std::string json = "{" + fmt::format("'odi_high': {}, 'oid_low': {}", high, low) + "}";
                return json;
            }
        };
    }
}
