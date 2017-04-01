#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file BunHelper.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Some helper function definitions for Bun.
///////////////////////////////////////////////////////////////////////////////

#include "blib/bun/SimpleOID.hpp"
#include <string>
#include <memory>
#include "blib/utils/MD5.hpp"

namespace blib{
    namespace bun{
        /////////////////////////////////////////////////
        /// @class BunHelper
        /// @brief Helper class for the persistent framework.
        ///        This class is specialized to persist objects.
        /////////////////////////////////////////////////
        template<typename T>
        struct BunHelper {
            inline static void createTable();
            inline static SimpleOID persistObj( T* );
            inline static void updateObj( T*, SimpleOID const& );
            inline static void deleteObj( SimpleOID const& );
            inline static std::unique_ptr<T> getObj( SimpleOID const& );
            inline static std::string md5( T*, SimpleOID const& );
            inline static std::string objToString( T*, SimpleOID const& );
            inline static std::string objToJson( T*, SimpleOID const& );
        };
    }
}