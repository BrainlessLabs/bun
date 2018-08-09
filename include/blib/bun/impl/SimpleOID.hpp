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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <third_party/fmt/format.hpp>

namespace blib {
    namespace bun {
        /// @class SimpleOID
        /// @brief Class to generate and hold a simple OID.
        /// @details The OID generation is very simple.
        ///          The OID is generated using boost UUID.
        struct SimpleOID {
            /// @typedef SelfType=SimpleOID
            using SelfType = SimpleOID;
            /// @typedef OidByteArrayType=std::array<std::uint8_t, 16>
            /// @brief To hold the 16 byte array representation of the OID.
            using OidByteArrayType = std::array<std::uint8_t, 16>;
			/// @typedef TagType = boost::uuids::uuid
			/// @brief Type to hold oid value
			using TagType = boost::uuids::uuid;
            /// @var TagType tag
            /// @brief Holds the UUID generated.
			TagType tag;

			SimpleOID() noexcept : tag() {}

            ~SimpleOID() = default;

            /// @fn SimpleOID
            /// @brief Pass other oid to populate this.
            /// @param other Passed in oid
            SimpleOID(const SimpleOID& other) :
                    tag(other.tag) {
            }

			SimpleOID(const std::string& other) {
				boost::uuids::string_generator gen;
				tag = gen(other);
			}

            /// @fn populateAll
            /// @brief Gets a new UUID and populates that.
            ///        Uses boost::uuid library for the task
            void populate() {
				tag = boost::uuids::random_generator()();
            }

            /// @fn clear
            /// @brief clears the high and low values.
            void clear() {
				for (boost::uuids::uuid::iterator it = tag.begin(); it != tag.end(); ++it) {
					*it = 0;
				}
            }

            /// @fn operator=
            /// @brief Copies the value of high and low from the RHS. Deep copy
            SelfType& operator=(SelfType const &in_other) {
                tag = in_other.tag;
                return *this;
            }

			/// @fn operator=
			/// @brief Copies the value of high and low from the RHS. Deep copy
			SelfType& operator=(std::string const &in_other) {
				boost::uuids::string_generator gen;
				tag = gen(in_other);
				return *this;
			}

            /// @fn operator==
            /// @brief Compares high and low of the LHS and RHS.
            ///        Returns true when both low and high are same.
            /// @return true when both low and high are same else false
            bool operator==(SelfType const &in_other) const {
                return tag == in_other.tag;
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
                OidByteArrayType ret{0};
				std::copy(tag.begin(), tag.end(), ret.begin());
                return ret;
            }
            
            /// @fn to_json
            /// @brief Returns the json representation of the UUID
            std::string to_json() const {
                const std::string json = "{" + fmt::format("'oid': '{}'", to_string()) + "}";
                return json;
            }

            /// @fn to_string
            /// @brief Returns the string representation of the UUID
			std::string to_string() const {
				const std::string ret = boost::uuids::to_string(tag);
				return ret;
			}
			
			/// @fn empty
			/// @brief Returns true if the oid is nil else returns false
			bool empty() const {
				return tag.is_nil();
			}
        };
    }
}
