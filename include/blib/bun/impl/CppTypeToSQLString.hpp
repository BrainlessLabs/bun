#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file CppTypeToSQLString.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Meta function to Map C++ types to SQL types
///////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>

namespace blib {
    namespace bun {
        /// @class DbTypes
        /// @brief This Enum gives all the types that are supported as of now.
        /// @details For any C++ type that needs to be supported it has to be
        ///          added here and then the Class specialization needs to be
        ///          done.
        enum class DbTypes : std::uint32_t {
            kUnknown = 0,
            kInteger,
            kReal,
            kNumeric,
            kText,
			kComposite
        };

        /// @class CppTypeToDbType
        /// @brief This class needs to be specialized for each of the C++ type
        ///         that needs to be supported.
        /// @details The specialize metafunction returns a type enum that will
        ///          be used by functions overload to return the proper type
        ///          The default specialization is unknown.
        template<typename T>
        struct CppTypeToDbType {
            /// @var static const DbTypes
            /// @brief Holds one of the values of the enum DbTypes
            static const DbTypes ret = DbTypes::kUnknown;
        };
        
        template<typename T>
        struct CppTypeToDbType<std::vector<T>>{
            /// @brief All the vectors are converted to strings
            static const DbTypes ret = DbTypes::kInteger;
		};

        template<>
        struct CppTypeToDbType<int> {
            static const DbTypes ret = DbTypes::kInteger;
        };

        template<>
        struct CppTypeToDbType<unsigned int> {
            static const DbTypes ret = DbTypes::kInteger;
        };

        template<>
        struct CppTypeToDbType<char> {
            static const DbTypes ret = DbTypes::kInteger;
        };

        template<>
        struct CppTypeToDbType<unsigned char> {
            static const DbTypes ret = DbTypes::kInteger;
        };

        template<>
        struct CppTypeToDbType<float> {
            static const DbTypes ret = DbTypes::kReal;
        };

        template<>
        struct CppTypeToDbType<double> {
            static const DbTypes ret = DbTypes::kReal;
        };

        template<>
        struct CppTypeToDbType<std::string> {
            static const DbTypes ret = DbTypes::kText;
        };

        template<>
        struct CppTypeToDbType<std::wstring> {
            static const DbTypes ret = DbTypes::kText;
        };

        /// @fn cppTypeEnumToDbTypeString
        /// @brief Convert cpp type to sqlite type. Function needs to be
        ///        specialized to that type.
        /// @return std::string Returns the SQL Type equivalent.
        template<DbTypes N = DbTypes::kUnknown>
        inline std::string const& cppTypeEnumToDbTypeString() {
            const static std::string ret = "UNKNOWN";
            return ret;
        }

        template<>
        inline std::string const &cppTypeEnumToDbTypeString<DbTypes::kInteger>() {
            const static std::string ret = "INTEGER";
            return ret;
        }

        template<>
        inline std::string const &cppTypeEnumToDbTypeString<DbTypes::kNumeric>() {
            const static std::string ret = "NUMERIC";
            return ret;
        }

        template<>
        inline std::string const &cppTypeEnumToDbTypeString<DbTypes::kReal>() {
            const static std::string ret = "REAL";
            return ret;
        }

        template<>
        inline std::string const &cppTypeEnumToDbTypeString<DbTypes::kText>() {
            const static std::string ret = "VARCHAR";
            return ret;
        }

		template<>
		inline std::string const &cppTypeEnumToDbTypeString<DbTypes::kComposite>() {
			const static std::string ret = "__COMPOSITE__";
			return ret;
		}

        /// @fn cppTypeToDbTypeString
        /// @brief This is the final piece of puzzle in the type conversion.
        /// @details This function call the previous function specialization
        ///          which in turn calls the meta function to get the enum.
        template<typename T>
        inline std::string const &cppTypeToDbTypeString() {
            return cppTypeEnumToDbTypeString<CppTypeToDbType<T>::ret>();
        }
    }
}
