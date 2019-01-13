#include "blib/bun/bun.hpp"
#include <boost/core/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/preprocessor.hpp>
#include <boost/proto/proto.hpp>
#include <soci/soci.h>
#include <string>
#include <soci/sqlite3/soci-sqlite3.h>
#include <boost/preprocessor.hpp>
#include <third_party/fmt/format.hpp>
#include <third_party/xxhash/xxhash.hpp>

using namespace soci;
using namespace std;

namespace bakery {
    struct A {
        int i;
        A(const int i = -1):i(i) {};
    };

	struct B {
		std::vector<int> i;
		std::string j;
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
        A a;
        Bun() :bun_name(), sugar_quantity(123.12), bun_length(77), a(12) {}
    };
}

SPECIALIZE_BUN_HELPER((bakery::A, i))
SPECIALIZE_BUN_HELPER((bakery::B, i, j));
SPECIALIZE_BUN_HELPER((bakery::Bun, bun_name, sugar_quantity, bun_length, json, a));

struct Child {
    int cf1;
    Child(const int cf = -1) : cf1(cf) {}
    Child& operator=(const int i) {
        cf1 = i;
        return *this;
    }
};

struct Parent {
    int f1;
    std::string f2;
    Child f3;
    Parent() :f1(-1), f2("-1"), f3(-1) {}
};

SPECIALIZE_BUN_HELPER((Child, cf1));
SPECIALIZE_BUN_HELPER((Parent, f1, f2, f3));

struct Person
{
    std::string id;
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
    oid.populate();
    Person p;
    p.id = oid.to_string();
    p.lastName = "Smith";
    p.firstName = "Pat";
    blib::bun::__private::DbBackend<>::i().session() << "CREATE TABLE IF NOT EXISTS person (id BIGINT PRIMARY KEY, first_name VARCHAR, last_name VARCHAR)";
    blib::bun::__private::DbBackend<>::i().session() << "insert into person(id, first_name, last_name) "
        "values(:ID, :FIRST_NAME, :LAST_NAME)", use(p);

    Person p1;
    blib::bun::__private::DbBackend<>::i().session() << "select * from person", into(p1);
    //assert(p1.id == 1);
    assert(p1.firstName + p.lastName == "PatSmith");
    assert(p1.gender == "unknown");

    p.firstName = "Patricia";
    blib::bun::__private::DbBackend<>::i().session() << "update person set first_name = :FIRST_NAME "
        "where id = :ID", use(p);
}

int ormTest() {
    namespace bun = blib::bun;
    namespace query = blib::bun::query;

    auto str = blib::bun::__private::SqlString<bakery::Bun>::create_table_sql();
    str = blib::bun::__private::SqlString<bakery::Bun>::insert_row_sql();
    const std::string conn_str =
#if defined(BUN_SQLITE)
        "obj.db";
#elif defined(BUN_POSTGRES)
        "postgresql://localhost/postgres?user=postgres&password=postgres";
#endif
    bun::connect("obj.db");

    try {
        //persistPerson();
    }
    catch (std::exception const & e) {
        blib::bun::l().error("{}", e.what());
        std::cout << e.what() << std::endl;
    }

    // We can also define constraints naturally as follows
    using AFields = query::F<bakery::A>;
    blib::bun::Configuration<bakery::A> a_config;
    // This is a unique key constrains thats applied.
    // Constraint are applied globally. They need to be set before the
    // execution of the create schema statement
    //a_config.set(AFields::i = blib::bun::unique_constraint);
    str = blib::bun::__private::SqlString<bakery::A>::create_table_sql();
    //blib::bun::connect("objects.db");
    blib::bun::createSchema<bakery::A>();
    blib::bun::createSchema<bakery::Bun>();

    blib::bun::PRef<bakery::Bun> bunn = new bakery::Bun;
    bunn->bun_length = 6;
    bunn->bun_name = "666";
    bunn->sugar_quantity = 66.6;
    bunn->json = "{666}";
    //bunn->a.i = 666;
    const auto oid = bunn.save();
    bunn->bun_length = 11;
    bunn->bun_name = "test";
    bunn->sugar_quantity = 55.6;
    bunn.save();

    // This will throw exceptions due to duplicate key.
    for (int i = 0; i < 1; ++i) {
        blib::bun::PRef<bakery::A> a = new bakery::A;
        a->i = 10;
    }

    std::cout << "How many objects to insert? " << std::endl;
    int count = 0;
    std::cin >> count;
    for (int i = 0; i < count; ++i) {
        blib::bun::l().info("===============Start===================");
        blib::bun::PRef<bakery::Bun> bunn = new bakery::Bun;
        bunn->bun_name = i % 2 ? "Delete Me" : "Do not Delete Me";
        bunn->bun_length = i;
        bunn->sugar_quantity = 55.6 * i;
        bunn->json = "";
        bunn->json = fmt::format("{}", bunn.toJson());
        //bunn->a.i = i * 13;

        const auto oid = bunn.save();
        std::cout << "Added to db: \n" << bunn.toJson() << std::endl;
        blib::bun::l().info("===============End===================\n");
    }

    blib::bun::createSchema<Child>();
    blib::bun::createSchema<Parent>();
    std::cout << "How many objects to insert? " << std::endl;

    std::cin >> count;
    for (int i = 0; i < count; ++i) {
        blib::bun::l().info("===============Start===================");
        blib::bun::PRef<Parent> p = new Parent;
        p->f1 = i;
        p->f2 = i % 2 ? "Delete Me" : "Do not Delete Me";
        p->f3 = 10 * i;
        p.persist();
        //blib::bun::SimpleOID oid;
        //oid.populate();
        //std::cout << oid.to_string() << std::endl;
        std::cout << "Added to db: \n" << p.toJson() << std::endl;
        blib::bun::l().info("===============End===================\n");
    }

    std::cout << "Get all objects and show" << std::endl;
    auto parents = blib::bun::getAllObjects<Parent>();
    for (auto p : parents) {
        std::cout << p.toJson() << std::endl;
        p.del();
    }

    using BunFields = query::F<bakery::Bun>;
    using FromBun = query::From<bakery::Bun>;
    FromBun fromBun;

    auto valid_query = BunFields::bun_length > 1 && BunFields::bun_name == "Delete Me";


    std::cout << "Press any key to delete" << std::endl;
    char c;
    std::cin >> c;
    auto where = fromBun.where(valid_query);
    auto buns = fromBun.where(valid_query).objects();
    //while (where.hasNext()) {
        //std::cout << where.next().toJson() << std::endl;
    //}
    for (auto bun : buns) {
        std::cout << "Delete from db: \n" << bun.toJson() << std::endl;
        bun.del();
    }

    return 1;
}

int kvTest() {
    blib::bun::KVDb<> db("kv.db");
    db.put("test", "test");
    std::string val;
    db.get("test", val);
    std::cout << val << std::endl;

    const int size = 3;
    for (int i = 0; i < size; ++i) {
        const std::string s = fmt::format("storing number: {}", i);
        db.put(i, s);
    }

    std::cout << "Start iteration Via size "<< std::endl;
    for (int i = 0; i < size; ++i) {
        std::string val;
        db.get(i, val);
        std::cout << val << std::endl;
    }

    std::cout << "Start iteration via foreach "<< std::endl;
	int count = 0;
    for (auto kv : db) {
        int key = 0;
        blib::bun::from_byte_vec(kv.first, key);

		std::string value;
		blib::bun::from_byte_vec(kv.second, value);
        std::cout << count++ << ")> key: "<< key << "\n Value: " << value << std::endl;
    }

    return 1;
}

namespace dbg {
	struct C1 {
		int c1;
		C1() :c1(2) {}
	};

    struct C {
        int c;
		C1 c1;
        C(const int i = 1) :c(i) {}
    };

    struct P {
        std::string p;
        C c;
        P() :p("s1"), c(1) {}
    };
}
SPECIALIZE_BUN_HELPER((dbg::C1, c1));
SPECIALIZE_BUN_HELPER((dbg::C, c, c1));
SPECIALIZE_BUN_HELPER((dbg::P, p, c));

int jsonTest() {
	namespace bun = blib::bun;

	blib::bun::PRef<dbg::P> p = new dbg::P;
	p->p = "s11";
	p->c.c = 10;

	p->c.c1.c1 = 12;


	blib::bun::PRef<dbg::C> c = new dbg::C;
	c->c = 666;
	// Convert the object to JSON
	const std::string json_string = p.toJson();
	// Construct the new object out of JSON
	blib::bun::PRef<dbg::P> p1;
	p1.fromJson(json_string);
	const auto msgpack = p1.toMesssagepack();
	// Construct another object out of messagepack
	blib::bun::PRef<dbg::P> p2;
	p2.fromMessagepack(p1.toMesssagepack());
	// messagepack to string
	std::string msgpack_string;
	for (auto c : msgpack) {
		msgpack_string.push_back(c);
	}

	std::cout << "1. Original object Object:" << json_string << std::endl;
	std::cout << "2. Object from JSON      :" << p1.toJson() << std::endl;
	std::cout << "3. Object to Messagepack :" << msgpack_string << std::endl;
	std::cout << "4. Object from Messagepck:" << p2.toJson() << std::endl;
	std::cout << "=== Vector JSON Conversion ===" << std::endl;
	blib::bun::PRef<bakery::B> b = new bakery::B;
	b->j = "test";
	b->i.push_back(12);
	b->i.push_back(23);
	std::cout << "5. Object with Vector: " << b.toJson() << std::endl;
	blib::bun::PRef<bakery::B> b1 = new bakery::B;
	b1.fromJson(b.toJson());
	std::cout << "6. Object copy with Vector: " << b1.toJson();
	return 1;
}

int ormDbg() {
    namespace bun = blib::bun;
    namespace query = blib::bun::query;

#if defined(BUN_SQLITE)
    bun::connect("obj.db");
#elif defined(BUN_POSTGRES)
    bun::connect("postgresql://localhost/postgres?user=postgres&password=postgres");
#endif
    // We can also define constraints naturally as follows
    using CFields = query::F<dbg::C>;
    blib::bun::Configuration<dbg::C> a_config;
    // This is a unique key constrains thats applied.
    // Constraint are applied globally. They need to be set before the
    // execution of the create schema statement
    //a_config.set(CFields::c = blib::bun::unique_constraint);
    //blib::bun::connect("objects.db");
    blib::bun::l().info("===============createSchema Start===================\n");
    blib::bun::createSchema<dbg::C>();
    blib::bun::createSchema<dbg::P>();
    blib::bun::l().info("===============createSchema End===================\n");
    blib::bun::PRef<dbg::P> p = new dbg::P;
    p->p = 1;
    p->c.c = 0;
    blib::bun::l().info("===============save p Start===================\n");
    p.save();
    blib::bun::l().info("===============save p End===================\n");
    std::cout << "Deleting: " << p.toJson() << std::endl;
    p.del();
    for (int i = 0; i < 5; i++) {
        blib::bun::PRef<dbg::P> p = new dbg::P;
        p->p = i * i;
        p->c.c = i;
        p.save();
    }
    using PFields = query::F<dbg::P>;
    auto valid_query = PFields::p > 1;
    using FromP = query::From<dbg::P>;
    FromP fromp;
    auto where = fromp.where(valid_query);
    while (where.hasNext()) {
        std::cout << where.next().toJson() << std::endl;
    }
    return 1;
}

int ormDbgSimple() {
    namespace bun = blib::bun;
    namespace query = blib::bun::query;

#if defined(BUN_SQLITE)
    bun::connect("obj.db");
#elif defined(BUN_POSTGRES)
    bun::connect("postgresql://localhost/postgres?user=postgres&password=postgres");
#endif
    // We can also define constraints naturally as follows
    using CFields = query::F<dbg::C>;
    //blib::bun::Configuration<dbg::C> a_config;
    blib::bun::l().info("===============createSchema Start===================\n");
    blib::bun::createSchema<dbg::C>();
    blib::bun::l().info("===============createSchema End===================\n");
    int count = 0;
    std::cout << "How many objects to insert? " << std::endl;
    std::cin >> count;
    for (int i = 0; i < count; i++) {
        blib::bun::PRef<dbg::C> c = new dbg::C;
        c->c = i;
        blib::bun::l().info("===============save p Start===================\n");
        c.save();
        blib::bun::l().info("===============save p End===================\n");
    }
    using CFields = query::F<dbg::C>;
    auto valid_query = CFields::c > 0;
    using FromC = query::From<dbg::C>;
    FromC from;
    auto where = from.where(valid_query);
    while (where.hasNext()) {
        std::cout << "next: " << where.next().toJson() << std::endl;
    }

    auto it = where.begin();
    auto ite = where.end();
    std::cout << it->toJson() << std::endl;
    for (auto it = where.begin();
         it != ite;
         ++it) {
        auto v = *it;
        std::cout << "Itr: " << v->c << std::endl;
    }

    for(auto v: where){
        std::cout << "foreach: " <<v->c << std::endl;
    }
    return 1;
}

int main() {
    namespace bun = blib::bun;
	//rapidjson::Document d;
	//rapidjson::Value v = d.GetObjectW();
	jsonTest();
    //kvTest();
    //ormTest();
    //ormDbg();
    //ormDbgSimple();
    return 1;
}
