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
		A(const int i) :i(i) {};
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

namespace bakery {
	struct Bun {
		std::string bun_name;
		double sugar_quantity;
		int bun_length;
		std::string json;
		int a;
		Bun() :bun_name(), sugar_quantity(123.12), bun_length(77), a(12) {}
	};
}

SPECIALIZE_BUN_HELPER((bakery::A, i));
SPECIALIZE_BUN_HELPER((bakery::Bun, bun_name, sugar_quantity, bun_length, json, a));

struct Person
{
	int id;
	std::string firstName;
	std::string lastName;
	std::string gender;
};

namespace soci
{
	template<>
	struct type_conversion<Person>
	{
		typedef values base_type;

		static void from_base(values const & v, indicator /* ind */, Person & p)
		{
			p.id = v.get<int>("ID");
			p.firstName = v.get<std::string>("FIRST_NAME");
			p.lastName = v.get<std::string>("LAST_NAME");

			// p.gender will be set to the default value "unknown"
			// when the column is null:
			p.gender = v.get<std::string>("GENDER", "unknown");

			// alternatively, the indicator can be tested directly:
			// if (v.indicator("GENDER") == i_null)
			// {
			//     p.gender = "unknown";
			// }
			// else
			// {
			//     p.gender = v.get<std::string>("GENDER");
			// }
		}

		static void to_base(Person & p, values & v, indicator & ind)
		{
			v.set("ID", p.id);
			v.set("FIRST_NAME", p.firstName);
			v.set("LAST_NAME", p.lastName);
			v.set("GENDER", p.gender, p.gender.empty() ? i_null : i_ok);
			ind = i_ok;
		}
	};
}

void persistPerson() {
	blib::bun::SimpleOID oid;
	oid.populateLow();
	Person p;
	p.id = oid.low;
	p.lastName = "Smith";
	p.firstName = "Pat";
	blib::bun::__private::DbBackend<>::i().session() << "CREATE TABLE IF NOT EXISTS person (id BIGINT PRIMARY KEY, first_name VARCHAR, last_name VARCHAR)";
	blib::bun::__private::DbBackend<>::i().session() << "insert into person(id, first_name, last_name) "
		"values(:ID, :FIRST_NAME, :LAST_NAME)", use(p);

	Person p1;
	blib::bun::__private::DbBackend<>::i().session() << "select * from person", into(p1);
	assert(p1.id == 1);
	assert(p1.firstName + p.lastName == "PatSmith");
	assert(p1.gender == "unknown");

	p.firstName = "Patricia";
	blib::bun::__private::DbBackend<>::i().session() << "update person set first_name = :FIRST_NAME "
		"where id = :ID", use(p);
}

int main() {
	namespace bun = blib::bun;
	namespace query = blib::bun::query;

	//auto str = blib::bun::__private::SqlString<bakery::Bun>::create_table_sql();
	//str = blib::bun::__private::SqlString<bakery::Bun>::insert_row_sql();

	bun::connect("postgresql://localhost/postgres?user=postgres&password=postgres");
	try {
		//persistPerson();
	}
	catch (std::exception const & e) {
		blib::bun::l().error("{}", e.what());
		std::cout << e.what() << std::endl;
	}
	

	//blib::bun::connect("objects.db");
	blib::bun::createSchema<bakery::A>();
	blib::bun::createSchema<bakery::Bun>();

	blib::bun::PRef<bakery::Bun> bunn = new bakery::Bun;
	bunn->bun_length = 6;
	bunn->bun_name = "666";
	bunn->sugar_quantity = 66.6;
	bunn->json = "{666}";
	bunn->a = 666;
	const auto oid = bunn.save();
	bunn->bun_length = 11;
	bunn->bun_name = "test";
	bunn->sugar_quantity = 55.6;
	bunn.save();
	//blib::bun::SimpleOID oid1(1, 5582309293008);
	//blib::bun::PRef<bakery::Bun> bun1(oid1);
	//bun1.del();
	
	for (int i = 0; i < 10000; ++i) {
		blib::bun::PRef<bakery::Bun> bunn = new bakery::Bun;
		bunn->bun_name = i % 2 ? "Delete Me" : "Do not Delete Me";
		bunn->bun_length = i;
		bunn->sugar_quantity = 55.6 * i;
		bunn->json = "";
		bunn->json = fmt::format("{}", bunn.toJson());
		bunn->a = i * 13;
		const auto oid = bunn.save();
	}

	using BunFields = query::F<bakery::Bun>;
	using FromBun = query::From<bakery::Bun>;
	FromBun fromBun;

	auto valid_query = BunFields::bun_length > 1 && BunFields::bun_name == "Delete Me";
	
	/*
	std::cout << "Press any key to delete" << std::endl;
	char c;
	std::cin >> c;
	auto buns = fromBun.where(valid_query).objects();
	for (auto bun : buns) {
		bun.del();
	}
	*/
	return 1;
}
