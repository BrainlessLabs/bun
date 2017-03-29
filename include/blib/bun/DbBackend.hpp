#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file DBBackend.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Implements the database backend.
///////////////////////////////////////////////////////////////////////////////
#include <soci.h>
#include "blib/utils/Singleton.hpp"

#define BUN_SQLITE
#ifdef BUN_SQLITE

#include <sqlite3/soci-sqlite3.h>

#endif

namespace blib {
    namespace bun {
        namespace _private {
            struct DbTypeSqlite {
            };

            struct DbTypePostgres {
            };

            struct DbTypeMySql {
            };

            template<typename T>
            class DbBackend {
                bool ok();

                bool connect(const std::string const &in_params);

                soci::session &session();
            };
        }
    }
}

#ifdef BUN_SQLITE

#include <sqlite3/soci-sqlite3.h>

namespace blib {
    namespace bun {
        namespace _private {
            ///

            template<>
            class DbBackend<DbTypeSqlite> : blib::Singleton<DbBackend<DbTypeSqlite>> {
            public:
                soci::session session;

            private:
                bool _ok;

            private:
                DbBackend() : _ok(false) {
                }

            public:
                bool open(const std::string const &in_params) {
                    try {
                        session.open(soci::sqlite3, in_params);
                        _ok = true;
                    } catch (std::exception const &except) {
                        _ok = false;
                    }
                    return _ok;
                }

                bool ok() const {
                    return _ok;
                }
            };
        }
    }
}
#endif
