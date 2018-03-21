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
	struct A {
		int i;
		A() = default;
	};

	struct Bun1 {
		std::string bun_name;
		float sugar_quantity;
		float flour_quantity;
		float milk_quantity;
		float yeast_quantity;
		float butter_quantity;
		int bun_length;
		A a;
	};
}

SPECIALIZE_BUN_HELPER((backery::A, i));

namespace backery {
	struct Bun {
		std::string bun_name;
		double sugar_quantity;
		int bun_length;
		int a;
	};
}

SPECIALIZE_BUN_HELPER((backery::Bun, bun_name, sugar_quantity, bun_length, a));

int main() {
	namespace bun = blib::bun;
	namespace query = blib::bun::query;

	auto str = blib::bun::__private::SqlString<backery::Bun>::create_table_sql();
	str = blib::bun::__private::SqlString<backery::Bun>::insert_row_sql();

	bun::connect("postgresql://localhost/postgres?user=postgres&password=postgres");
	//blib::bun::connect("objects.db");
	blib::bun::createSchema<backery::Bun>();

	blib::bun::PRef<backery::Bun> bunn = new backery::Bun;
	const auto oid = bunn.save();
	bunn->bun_length = 11;
	bunn->bun_name = "test";
	bunn->sugar_quantity = 55.6;
	bunn.save();
	blib::bun::SimpleOID oid1(1, 24134930076892);
	blib::bun::PRef<backery::Bun> bun1(oid1);
	bun1.del();
	for (int i = 0; i < 1000; ++i) {
		blib::bun::PRef<backery::Bun> bunn = new backery::Bun;
		const auto oid = bunn.save();
		bunn->bun_length = i;
		bunn->sugar_quantity = 55.6 * i;
		bunn->bun_name = fmt::format("{}", bunn.toJson());
	}
	return 1;
}
