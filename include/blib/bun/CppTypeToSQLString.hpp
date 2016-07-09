#pragma once

#include <cstdint>
#include <cstdlib>

namespace blib {
  namespace bun {
    enum class DbTypes : std::uint32_t {
      kUnknown = 0,
      kInteger,
      kReal,
      kNumeric,
      kText
    };

    template<typename T>
    struct CppTypeToDbType {
      static const DbTypes ret = DbTypes::kUnknown;
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

    // Convert cpp type to sqlite type
    template<DbTypes N = DbTypes::kUnknown>
    inline std::string const& cppTypeEnumToDbTypeString() {
      const static std::string ret = "UNKNOWN";
      return ret;
    }

    template<>
    inline std::string const& cppTypeEnumToDbTypeString<DbTypes::kInteger>() {
      const static std::string ret = "INTEGER";
      return ret;
    }

    template<>
    inline std::string const& cppTypeEnumToDbTypeString<DbTypes::kNumeric>() {
      const static std::string ret = "NUMERIC";
      return ret;
    }

    template<>
    inline std::string const& cppTypeEnumToDbTypeString<DbTypes::kReal>() {
      const static std::string ret = "REAL";
      return ret;
    }

    template<>
    inline std::string const& cppTypeEnumToDbTypeString<DbTypes::kText>() {
      const static std::string ret = "TEXT";
      return ret;
    }

    template<typename T>
    inline std::string const& cppTypeToDbTypeString() {
      return cppTypeEnumToDbTypeString<CppTypeToDbType<T>::ret>();
    }
  }
}