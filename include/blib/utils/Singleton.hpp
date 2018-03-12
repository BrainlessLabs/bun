#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file Singleton.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Helper class for creating a singleton.
///////////////////////////////////////////////////////////////////////////////

#include <boost/core/noncopyable.hpp>

namespace blib {
    /////////////////////////////////////////////////
    /// @class Singleton
    /// @brief Helper class to create a singleton.
    /// @details Example usage:
    ///          struct MakeMeSingleton : public ::blib::Singleton<MakeMeSingleton>
    ///          {};
    /////////////////////////////////////////////////
    template<class T>
    class Singleton : public boost::noncopyable {
    public:
        static T &i() {
            static T _me;
            return _me;
        }
    };
}