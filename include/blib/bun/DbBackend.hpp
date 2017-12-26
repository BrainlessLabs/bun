#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file DBBackend.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Implements the database backend.
///////////////////////////////////////////////////////////////////////////////
#include <soci/soci.h>
#include "blib/utils/Singleton.hpp"
#include "blib/bun/DbLogger.hpp"
#include <memory>

#define BUN_SQLITE

#ifdef BUN_SQLITE

#include <soci/sqlite3/soci-sqlite3.h>

#elif BUN_POSTGRES
#include <soci/postgresql/soci-postgresql.h>
#elif BUN_MYSQL
#include <soci/mysql/soci-mysql.h>
#endif


namespace blib {
    namespace bun {
        namespace __private {
            struct DbGenericType{
            };
            
            struct DbTypeSqlite : DbGenericType{
            };

            struct DbTypePostgres : DbGenericType{
            };

            struct DbTypeMySql : DbGenericType{
            };
            

            template<typename T = DbGenericType>
            class DbBackend : public blib::Singleton<DbBackend<T>> {
            private:
                bool _ok;
                soci::session _sql_session;

                //DbBackend() = default;

            public:
                bool ok() const {
                    return _ok;
                }

                bool connect(std::string const &in_params) {
                    const auto backend_factory =
#ifdef BUN_SQLITE
                    soci::sqlite3;
#elif BUN_POSTGRES
                    soci::postgresql;
#elif BUN_MYSQL
                    soci::mysql;
#endif
                    try {
						if (!_ok) {
							_sql_session.open(backend_factory, in_params);
							_ok = true;
						}
                    }
                    catch (std::exception const &except) {
						l().error(except.what());
                        _ok = false;
                    }
					return _ok;
                }

                soci::session &session() {
                    return _sql_session;
                }
            };

			/////////////////////////////////////////////////
			/// @class ConvertToSOCIType
			/// @brief Convert a basic type to a type available for soci.
			/// @details Supported type can be found in http://soci.sourceforge.net/doc/master/types/
			///          So all basic C++ types will be converted to closest type. By default same type is returned.
			/////////////////////////////////////////////////
			template<typename T>
			struct ConvertToSOCIType {
				using type = T;
			};

			template<>
			struct ConvertToSOCIType<float> {
				using type = double;
			};

			/// @brief Works for all stuff where the default type conversion operator is overloaded.
			template<typename T>
			typename ConvertToSOCIType<T>::type convertToSOCIType(T const & val) {
				return (ConvertToSOCIType<T>::type)(val);
			}

			std::string convertToSOCIType( char const* val) {
				std::string ret = val;
				return std::move(ret);
			}
        }
    }
}
