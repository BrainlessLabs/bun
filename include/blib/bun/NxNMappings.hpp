#include <vector>
#include <string>
#include "blib/bun/CppTypeToSQLString.hpp"

namespace blib{
    namespace bun{
        namespace __private{
            template<typename T>
            struct OneToNCreation{
                std::string parent_table(){
                    return "";
                }
                
                std::string table_name(){
                    return "";
                }
                
                std::string sql(){
                    return "";
                }
                
                private:
                std::string __parent_table(const std::string& /*parent_table*/){
                    return "";
                }
                
                std::string __table_name(const std::string& /*table-name*/){
                    return "";
                }
            }
            
            template<typename T>
            struct OneToNCreation<std::vector<T>>{
                std::string parent_table(){
                    return "";
                }
                
                std::string table_name(){
                    return "";
                }
                
                std::string sql(){
                    return "";
                }
                
                private:
                std::string __parent_table(const std::string& parent_table){
                    return "";
                }
                
                std::string __table_name(const std::string& /*table-name*/){
                    return "";
                }
            }
        }
    }
}