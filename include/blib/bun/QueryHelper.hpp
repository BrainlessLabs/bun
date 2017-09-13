#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file QueryHelper.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Some helper function definitions for PRef
///////////////////////////////////////////////////////////////////////////////

#include "blib/bun/DbBackend.hpp"
#include "blib/bun/SimpleOID.hpp"
#include "blib/bun/DbLogger.hpp"
#include "blib/bun/PRef.hpp"
#include "blib/bun/CppTypeToSQLString.hpp"
#include <string>
#include <vector>

namespace blib {
    namespace bun {
        namespace __private {
            /////////////////////////////////////////////////
            /// @class QueryHelper
            /// @brief Helper class for the persistent framework.
            ///        This class is specialized to persist objects.
            /////////////////////////////////////////////////
            template<typename T>
            struct QueryHelper {
                inline static std::vector <SimpleOID> getAllOids();

                inline static std::vector <PRef<T>> getAllObjects();

                inline static std::vector <blib::bun::PRef<T>> getAllObjWithQuery(std::string const &in_query);
            };
        }
    }
}