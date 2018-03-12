#include <boost/core/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor.hpp>
#include <boost/proto/proto.hpp>
#include <soci/soci.h>
#include <string>
#include <soci/sqlite3/soci-sqlite3.h>
#include <boost/preprocessor.hpp>
#include <third_party/fmt/format.hpp>
#include "blib/bun/bun.hpp"

using namespace soci;
using namespace std;

namespace backery {
	struct Bun1 {
		std::string bun_name;
		float sugar_quantity;
		float flour_quantity;
		float milk_quantity;
		float yeast_quantity;
		float butter_quantity;
		int bun_length;
	};
}

namespace backery {
	struct Bun {
		std::string bun_name;
		double sugar_quantity;
		int bun_length;
	};
}

SPECIALIZE_BUN_HELPER((backery::Bun, bun_name, sugar_quantity, bun_length));

int main() {
	auto str = blib::bun::__private::SqlString<backery::Bun>::create_table_sql();
	str = blib::bun::__private::SqlString<backery::Bun>::insert_row_sql();

	blib::bun::connect("objects.db");
	blib::bun::createSchema<backery::Bun>();

	blib::bun::PRef<backery::Bun> bun = new backery::Bun;
	const auto oid = bun.save();
	bun->bun_length = 11;
	bun->bun_name = "test";
	bun->sugar_quantity = 55.6;
	bun.save();
	blib::bun::SimpleOID oid1(1, 24134930076892);
	blib::bun::PRef<backery::Bun> bun1(oid1);
	bun1.del();
	return 1;
}
