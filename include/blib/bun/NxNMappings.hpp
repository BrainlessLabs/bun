#include <vector>
#include <string>
#include "blib/bun/CppTypeToSQLString.hpp"

namespace blib{
    namespace bun{
        namespace __private{
            template<typename T>
            struct NxNMappingTables{
                static std::string base_table_name();
                static std::string table_name();
            }
            
            template<typename T>
            struct OneToNCreationSql{
                static std::string sql(){
                    return "";
                }
            }
            
            template<typename T>
            struct OneToNCreationSql<std::vector<T>>{
                static std::string const& table_name(){
                    static const std::string table_name = NxNMappingTables<std::vector<T>>::base_table_name() + "_" + NxNMappingTables<std::vector<T>>::table_name()
                    return table_name;
                }
                
                static std::string sql(){
                    static std::string sql = "CREATE TABLE '" + table_name() + "' WITH "
                }
            }
        }
    }
}