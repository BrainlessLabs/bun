#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file DBBackend.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Implements the database backend.
///////////////////////////////////////////////////////////////////////////////
#include <soci.h>
#include "blib/utils/Singleton.hpp"
#include "blib/bun/DbLogger.hpp"
#include <memory>

#define BUN_SQLITE

#ifdef BUN_SQLITE
#include <sqlite3/soci-sqlite3.h>
#elif BUN_POSTGRES
#include <postgresql/soci-postgresql.h>
#elif BUN_MYSQL
#include <mysql/soci-mysql.h>
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
            class DbBackend : blib::Singleton<DbBackend<T>>{
            private:
                bool _ok;
                soci::session _sql_session;

                DbBackend() = default;

            public:
                bool ok() const{
                    return _ok;
                }

                bool connect(const std::string const &in_params){
                    const soci::backend_factory backend_factory =
#ifdef BUN_SQLITE
                    soci::sqlite3;
#elif BUN_POSTGRES
                    soci::postgresql;
#elif BUN_MYSQL
                    soci::mysql;
#endif
                    try{
                        _sql_session.open(backend_factory, in_params);
                        _ok = true;
                    }
                    catch (std::exception const &except){
                        _ok = false;
                    }
                }

                soci::session &session(){
                    return _sql_session;
                }
            };
        }
    }
}
