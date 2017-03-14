#pragma once

#include <cstdint>
#include <type_traits>
#include <string>

namespace blib {
  namespace bun {
    /////////////////////////////////////////////////
    /// @class IsPersistant
    /// @brief If IsPersistant<Class>::value == true then Class can be persisted.
    /////////////////////////////////////////////////
    template<typename T>
    struct IsPersistant : std::integral_constant<bool, std::is_arithmetic<T>::value> {
    };

    template<>
    struct IsPersistant<std::string> : std::true_type {
    };
  }
}
