// bun.cpp : Defines the entry point for the console application.
//

#include "blib\bun\DbBackend.hpp"
#include <boost/core/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor.hpp>
#include <boost/proto/proto.hpp>
#include "blib/utils/MD5.hpp"
#include <soci/soci.h>
#include <string>
#include <soci/sqlite3/soci-sqlite3.h>
#include <boost/preprocessor.hpp>
#include <third_party/fmt/format.hpp>
#include "blib/bun/DbLogger.hpp"
#include "blib/bun/bun.hpp"

using namespace soci;
using namespace std;

struct Bun {
	std::string bun_name;
	float sugar_quantity;
	float flour_quantity;
	float milk_quantity;
	float yeast_quantity;
	float butter_quantity;
	int bun_length;
};

//SPECIALIZE_BUN_HELPER((Bun, bun_name, sugar_quantity));

int main() {
	try {
		session sql(sqlite3, "database_filename.db");
		blib::bun::_private::DbBackend().i().connect("");
		connection_parameters c;
		int count;
		sql << "select * from sqlite_master";
	}
	catch (exception const & e) {
		cerr << "Error: " << e.what() << "\n";
	}

	auto t = BOOST_PP_TUPLE_ELEM(5, (1, 2, 3, 4, 5, 6));
	std::cout << "Tuple Out: " << t << std::endl;
	blib::bun::l().info("test");

	return 1;
}
