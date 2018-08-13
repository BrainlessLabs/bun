# bun
## Description
Bun is a simple to use C++ Object Relational Mapper (ORM) library

[Bun Page](http://brainlesslabs.github.io/bun/)

[![OpenHub](https://www.openhub.net/p/brainlesslabs_bun/widgets/project_thin_badge.gif)](https://www.openhub.net/p/brainlesslabs_bun)

[![stable](http://badges.github.io/stability-badges/dist/stable.svg)](http://github.com/badges/stability-badges)

# Articles

## Introduction to bun

[CodeProject Article](http://www.codeproject.com/Tips/1100449/Cplusplus-Object-Relational-Mapping-ORM-Eating-the)


# Usage

```C++
#include "blib/bun/bun.hpp"

namespace test {
  // Class that needs to be persisted
  struct Person {
    std::string name;
    std::string uname;
    int age;
    float height;
  };
}

/// @class Child
struct Child {
    int cf1;
    Child(const int cf = -1) : cf1(cf) {}
    Child& operator=(const int i) {
        cf1 = i;
        return *this;
    }
};

/// @class Paret
struct Parent {
    int f1;
    std::string f2;
    // Nested object
    Child f3;
    Parent() :f1(-1), f2("-1"), f3(-1) {}
};

// Both should be persistable
SPECIALIZE_BUN_HELPER((Child, cf1));
SPECIALIZE_BUN_HELPER((Parent, f1, f2, f3));

/////////////////////////////////////////////////
/// Generate the database bindings at compile time.
/////////////////////////////////////////////////
SPECIALIZE_BUN_HELPER( (test::Person, name, uname, age, height) );

int main() {
  namespace bun = blib::bun;
  namespace query = blib::bun::query;

  // Connect the db. If the db is not there it will be created.
  // It should include the whole path
  // For SQLite
  //bun::connect( "objects.db" );
  // For PostGres
#if defined(BUN_SQLITE)
    bun::connect("obj.db");
#elif defined(BUN_POSTGRES)
    bun::connect("postgresql://localhost/postgres?user=postgres&password=postgres");
#endif
  // Get the fields of the Person. This will be useful in specifying constraints and also
  // querying the object.
  using PersonFields = query::F<test::Person>;

  // Generate the configuration. By default it does nothing.
  blib::bun::Configuration<test::Person> person_config;
  // This is a unique key constraints that is applied.
  // Constraint are applied globally. They need to be set before the
  // execution of the create schema statement
  // The syntax is Field name = Constraint
  // We can club multiple Constraints as below in the same statement.
  // There is no need for multiple set's to be called. This is how
  // We can chain different constraints in the same statement
  person_config.set(PersonFields::name = blib::bun::unique_constraint)
                   (PersonFields::uname = blib::bun::unique_constraint);

  // Create the schema. We can create the schema multiple times. If its already created
  // it will be safely ignored. The constraints are applied to the table.
  // Adding constraints don't have effect if the table is already created
  bun::createSchema<test::Person>();

  // Start transaction
  bun::Transaction t;
  // Create some entries in the database
  for (int i = 1; i < 1000; ++i) {
    // PRef is a reference to the persistent object.
    // PRef keeps the ownership of the memory. Release the memory when it is destroyed.
    // Internally it holds the object in a unique_ptr
    // PRef also has a oid associated with the object
    bun::PRef<test::Person> p = new test::Person;

    // Assign the members values
    p->age = i + 10;
    p->height = 5.6;
    p->name = fmt::format( "Brainless_{}", i );
    // Persist the object and get a oid for the persisted object.
    const bun::SimpleOID oid = p.persist();

    //Getting the object from db using oid.
    bun::PRef<test::Person> p1( oid );
  }
  // Commit the transaction
  t.commit();

  // To get all the object oids of a particular object.
  // person_oids is a vector of type std::vector<blib::bun<>SimpleOID<test::Person>>
  const auto person_oids = bun::getAllOids<test::Person>();

  // To get the objects of a particular type
  // std::vector<blib::bun::Pref<test::Person>>
  const auto person_objs = bun::getAllObjects<test::Person>();

  // EDSL QUERY LANGUAGE ----------------------
  // Powerful EDSL object query syntax that is checked for syntax at compile time.
  // The compilation fails at the compile time with a message "Syntax error in Bun Query"
  using FromPerson = query::From<test::Person>;
  FromPerson fromPerson;
  // Grammar are checked for validity of syntax at compile time itself.
  // Currently only &&, ||, <, <=, >, >=, ==, != are supported. They have their respective meaning
  // Below is a valid query grammar
  auto valid_query = PersonFields::age > 10 && PersonFields::name != "Brainless_0";
  std::cout << "Valid Grammar?: " << query::IsValidQuery<decltype(valid_query)>::value << std::endl;

  // Oops + is not a valid grammar
  auto invalid_query = PersonFields::age + 10 &&
  PersonFields::name != "Brainless_0";
  std::cout << "Valid Grammar?: " <<
  query::IsValidQuery<decltype(invalid_query)>::value << std::endl;

  // Now let us execute the query.
  // The where function also checks for the validity of the query, and fails at compile time
  const auto objs = fromPerson.where( valid_query ).where( valid_query ).objects();
  // Can even use following way of query
  // As you see we can join queries
  const auto q = PersonFields::age > 21 && PersonFields::name == "test";
  const auto objs_again = FromPerson().where( q ).objects();
  const auto objs_again_q = FromPerson().where( PersonFields::age > 21 && PersonFields::name == "test" ).objects();
  // Not going to compile if you enable the below line.
  // Will get the "Syntax error in Bun Query" compile time message.
  // const auto objs1 = FromPerson.where( invalid_query ).objects();

  // Check the query generated. It does not give the sql query.
  std::cout << fromPerson.query() << std::endl;

  // Support for Nested object persistence and retrieval
  blib::bun::createSchema<Child>();
  blib::bun::createSchema<Parent>();
  std::cout << "How many objects to insert? " << std::endl;
  int count = 0;
  std::cin >> count;
  for (int i = 0; i < count; ++i) {
      blib::bun::l().info("===============Start===================");
      blib::bun::PRef<Parent> p = new Parent;
      p->f1 = i;
      p->f2 = i % 2 ? "Delete Me" : "Do not Delete Me";
      p->f3 = 10 * i;
      // Persists the Parent and the Nested Child
      p.persist();
      std::cout << "Added to db: \n" << p.toJson() << std::endl;
      blib::bun::l().info("===============End===================\n");
    }

    std::cout << "Get all objects and show" << std::endl;
    auto parents = blib::bun::getAllObjects<Parent>();
    // Iterate and delete the Parent and the nested Child
    for (auto p : parents) {
        std::cout << p.toJson() << std::endl;
        p.del();
    }

    // Iterate the parent with range based for loop
    using FromParents = query::From<Parent>;
    using ParentFields = query::F<Parent>;
    FromParents from_parents;
    // Select the query which you want to execute
    auto parents_where = from_parents.where(ParentFields::f2 == "Delete Me");
    // Fetch all the objects satisfying the query. This is a lazy fetch. It will be fetched
    // only when it is called. And not all the objects are fetched.
    for(auto v : parents_where) {
        std::cout << v.toJson() << std::endl;
    }

    // ========= KV Store
    blib::bun::KVDb<> db("kv.db");

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
    count = 0;
    // Iterate the key value store using foreach
    for (auto kv : db) {
        int key = 0;
        blib::bun::from_byte_vec(kv.first, key);

        std::string value;
        blib::bun::from_byte_vec(kv.second, value);
        std::cout << count++ << ")> key: "<< key << "\n Value: " << value << std::endl;
    }


  return 0;
}
```
