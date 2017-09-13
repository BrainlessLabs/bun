#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file PRefHelper.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Some helper function definitions for PRef
///////////////////////////////////////////////////////////////////////////////

#include "blib/bun/SimpleOID.hpp"
#include "blib/utils/MD5.hpp"
#include <string>
#include <memory>

namespace blib{
    namespace bun{
        /////////////////////////////////////////////////
        /// @class PRefHelper
        /// @brief Helper class for the persistent framework.
        ///        This class is specialized to persist objects.
        /////////////////////////////////////////////////
        template<typename T>
        struct PRefHelper {
            inline static void createSchema();
            inline static void deleteSchema();
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
