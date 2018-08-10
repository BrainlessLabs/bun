#pragma once

///////////////////////////////////////////////////////////////////////////////
/// @file DBBackend.hpp
/// @author BrainlessLabs
/// @version 0.3
/// @brief Implements the database backend.
///////////////////////////////////////////////////////////////////////////////
#include <soci/soci.h>
#include "blib/utils/Singleton.hpp"
#include "blib/bun/impl/DbLogger.hpp"
#include <memory>

#ifdef BUN_SQLITE
#include <soci/sqlite3/soci-sqlite3.h>
#elif defined(BUN_POSTGRES)
#include <soci/postgresql/soci-postgresql.h>
#elif defined(BUN_MYSQL)
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
#elif defined(BUN_POSTGRES)
                    soci::postgresql;
#elif defined(BUN_MYSQL)
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
			struct ConvertCPPTypeToSOCISupportType {
				using type = T;
			};

			template<>
			struct ConvertCPPTypeToSOCISupportType<float> {
				using type = double;
			};

			template<>
			struct ConvertCPPTypeToSOCISupportType<std::uint64_t> {
				using type = unsigned long long;
			};

			/// @brief Works for all stuff where the default type conversion operator is overloaded.
			template<typename T>
			typename ConvertCPPTypeToSOCISupportType<T>::type convertToSOCISupportedType(T const& val) {
                                const auto ret = static_cast<typename ConvertCPPTypeToSOCISupportType<T>::type>(val);
				return ret;
			}

			std::string convertToSOCISupportedType( char const* val) {
				const std::string ret = val;
                                return ret;
			}
        }

		using DbGenericType = __private::DbGenericType;

		using DbTypeSqlite = __private::DbGenericType;

		using DbTypePostgres = __private::DbGenericType;

		using DbTypeMySql = __private::DbGenericType;
    }
}
