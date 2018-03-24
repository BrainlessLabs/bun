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

namespace bakery {
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

SPECIALIZE_BUN_HELPER((bakery::A, i));

namespace bakery {
	struct Bun {
		std::string bun_name;
		double sugar_quantity;
		int bun_length;
		std::string json;
		int a;
		Bun() :bun_name(), sugar_quantity(123.12), bun_length(77), a() {}
	};
}

SPECIALIZE_BUN_HELPER((bakery::Bun, bun_name, sugar_quantity, bun_length, json, a));

int main() {
	namespace bun = blib::bun;
	namespace query = blib::bun::query;

	auto str = blib::bun::__private::SqlString<bakery::Bun>::create_table_sql();
	str = blib::bun::__private::SqlString<bakery::Bun>::insert_row_sql();

	bun::connect("postgresql://localhost/postgres?user=postgres&password=postgres");
	//blib::bun::connect("objects.db");
	blib::bun::createSchema<bakery::Bun>();

	blib::bun::PRef<bakery::Bun> bunn = new bakery::Bun;
	const auto oid = bunn.save();
	bunn->bun_length = 11;
	bunn->bun_name = "test";
	bunn->sugar_quantity = 55.6;
	bunn.save();
	blib::bun::SimpleOID oid1(1, 5582309293008);
	blib::bun::PRef<bakery::Bun> bun1(oid1);
	bun1.del();
	
	for (int i = 0; i < 5; ++i) {
		blib::bun::PRef<bakery::Bun> bunn = new bakery::Bun;
		bunn->bun_name = i % 2 ? "Delete Me" : "Do not Delete Me";
		bunn->bun_length = i;
		bunn->sugar_quantity = 55.6 * i;
		bunn->json = "";
		bunn->json = fmt::format("{}", bunn.toJson());
		const auto oid = bunn.save();
	}

	using BunFields = query::F<bakery::Bun>;
	using FromBun = query::From<bakery::Bun>;
	FromBun fromBun;

	auto valid_query = BunFields::bun_length > 2 && BunFields::bun_name == "Delete Me";
	std::cout << "Press any key to delete" << std::endl;
	char c;
	std::cin >> c;
	auto buns = fromBun.where(valid_query).objects();
	for (auto bun : buns) {
		bun.del();
	}
	
	return 1;
}
