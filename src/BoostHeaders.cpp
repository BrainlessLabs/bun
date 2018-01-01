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

namespace backery {
	struct Bun {
		std::string bun_name;
		float sugar_quantity;
		float flour_quantity;
		float milk_quantity;
		float yeast_quantity;
		float butter_quantity;
		int bun_length;
	};
}

//SPECIALIZE_BUN_HELPER((backery::Bun, bun_name, sugar_quantity, bun_length));
#include "blib/utils/debug.hpp"

struct Person
{
	std::string name;
	int age;
	double account_balance;
	int gender;
	Person() :name("test"), age(10), account_balance(10.10), gender('M') {}
};

namespace soci
{
	template<>
	struct type_conversion<Person>
	{
		typedef values base_type;

		static void from_base(values const & v, indicator /* ind */, Person & p)
		{
			p.name = v.get<std::string>("name");
			p.age = v.get<int>("age");
			p.account_balance = v.get<double>("account_balance");
			p.gender = v.get<int>("gender");
		}

		static void to_base(const Person & p, values & v, indicator & ind)
		{
			v.set("name", p.name);
			v.set("age", p.age);
			v.set("account_balance", p.account_balance);
			v.set("gender", p.gender);
			ind = i_ok;
		}
	};
}

int main() {
	blib::bun::connect("objects.db");
	blib::bun::createSchema<backery::Bun>();
	try {
		session sql(sqlite3, "objects.db");
		connection_parameters c;
		int count = 0;
		int oid_low = 666;
		Person p;
		sql << "select * from sqlite_master";
		sql << "CREATE TABLE IF NOT EXISTS Person (oid_high INTEGER PRIMARY KEY AUTOINCREMENT, oid_low INTEGER, name VARCHAR, age INTEGER, account_balance REAL, gender INTEGER)";
		sql << "INSERT INTO 'backery::Bun' (oid_low,bun_name,sugar_quantity,bun_length) VALUES (666,\"test\",666, 666)";
		sql << "INSERT INTO Person (oid_low, name, age, account_balance, gender) VALUES(666, :name, :age, :account_balance, :gender)", soci::use(p);
		p.age = 12;
		sql << "UPDATE Person SET name =  :name, age = :age, account_balance = :account_balance, gender = :gender WHERE oid_low = 666 and oid_high = 2", soci::use(p);
		const auto t = BOOST_PP_TUPLE_ELEM(5, (1, 2, 3, 4, 5, 6));
		blib::bun::PRef<backery::Bun> bun = new backery::Bun;
		bun->bun_length = 10;
		bun->sugar_quantity = 10;
		bun->bun_name = "test";
		const std::string bun_json = bun.toJson();
		blib::bun::l().info("bun json: {}", bun_json);
		const auto oid = bun.save();
		bun->bun_length = 12;
		bun.save();
		std::cout << "Tuple Out: " << t << std::endl;
	}
	catch (exception const & e) {
		cerr << "Error: " << e.what() << "\n";
		blib::bun::l().error("SQL Error: {}", e.what());
	}

	return 1;
}
