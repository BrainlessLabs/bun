#include <boost/core/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor.hpp>
#include <boost/proto/proto.hpp>
#include "blib/utils/MD5.hpp"
#include <soci/soci.h>
#include <sqlite3/soci-sqlite3.h>

using namespace soci;
using namespace std;

int main(){
    try{
        session sql(sqlite3, "database_filename");
        connection_parameters c;
        int count;
        sql << "select * from sqlite_master";
    }
    catch(exception const & e){
        cerr << "Error: "<<e.what() << "\n";
    }

    std::cout << "Ending Program"<< std::endl;

    return 1;
}
