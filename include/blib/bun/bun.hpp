#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file Bun.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief The include file for Bun ORM. This file includes all the source
///        to make the class/struct persist in sqlite. Users need to include
///        only this file.
///////////////////////////////////////////////////////////////////////////////
#include <string>
#include <boost/preprocessor.hpp>
#include "blib/bun/BunHelper.hpp"
#include "blib/bun/DbBackend.hpp"
#include "blib/bun/DbLogger.hpp"

///////////////////////////////////////////////////////////////////////////////
/// @basic Basic Persistance Start
///////////////////////////////////////////////////////////////////////////////

/// @basic Create schema
/// @details Create the schema is essentially creating a table.
///          For each class there will be a different schema that will be created.

/// Helper Macros Start

/// Helper Macros End
/// SPECIALIZE_BUN_HELPER Start

#define SPECIALIZE_BUN_HELPER(CLASS_ELEMS_TUP) namespace blib{namespace bun{\
template<>\
struct BunHelper<BOOST_PP_TUPLE_ELEM()>{\
using ClassType = BOOST_PP_TUPLE_ELEM(0, CLASS_ELEMS_TUP);\
};\
}\
}\

SPECIALIZE_BUN_HELPER(())
/// SPECIALIZE_BUN_HELPER End


///////////////////////////////////////////////////////////////////////////////
/// @basic Basic Persistance End
///////////////////////////////////////////////////////////////////////////////