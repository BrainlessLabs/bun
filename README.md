# bun
## Description
Bun is a simple to use C++ Object Relational Mapper (ORM) library and much more (Key value store, JSON conversion and Message pack conversion). Following is the features of Bun:
* **Easy** to Use
* **Object persistence** - You can persist **C++ objects directly**.
* **Not intrusive** - You do not have to modify the classes to make it persistent
* **Constraint** Specification in plain C++
* Persist **Nested Objects**
* **C++ EDSL** Object Query Language (No SQL Query needed)
* **Compile time** EDSL syntax check for type safety - Catch bugs before the execution starts
* Multiple database support - SQLite, Postgres, MySQL
* Easy to use **embedded key-value** store
* Convert C++ objects to **JSON** and create C++ objects from JSON.
* Convert C++ objects to **Message Pack** and create C++ objects from Message Pack.
* **Lazy iteration** over the objects in the database using range based for loop in C++

[Bun Page](http://brainlesslabs.github.io/bun/)

[![OpenHub](https://www.openhub.net/p/brainlesslabs_bun/widgets/project_thin_badge.gif)](https://www.openhub.net/p/brainlesslabs_bun)

[![stable](http://badges.github.io/stability-badges/dist/stable.svg)](http://github.com/badges/stability-badges)

## Whats new
* ***Bun 1.4.0** Has support for converting objects to JSON and create Objects from JSON. It has the capabality to convert Objects to Message pack and construct object from message pack.
* **Bun 1.3** Has support for Object lazy iteration and ranges based for loop support. Same is supported for the key-value store too.
* **Bun 1.2** has support for embedded key-value store. But default, the key-value store is based on Unqlite.

# Articles

## Introduction to bun

[CodeProject Article](http://www.codeproject.com/Tips/1100449/Cplusplus-Object-Relational-Mapping-ORM-Eating-the)


# Usage
## Object Persistence
With Bun you can persist C++ obhects (POD) directly. The magic happens by registring the C++ structure/class with Bun and you are good to go.

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
  bun::connect("postgresql://localhost/postgres?user=postgres&password=postgres");
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
  const auto objs_again_q = FromPerson().where( PersonFields::age > 21 
  && PersonFields::name == "test" ).objects()
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
    // Here p is a PRef type. We can modify the object and persist 
    // the changes if needed.
    for (auto p : parents) {
        std::cout << p.toJson() << std::endl;
        p.del();
    }

  return 0;
}
```
### Range based iteration and Lazy iteration
Its easy to retrieve values from database and iterate over them. The iteration is a lazy iteration, that means that the values are not retrieved all at once but paginated.

```C++
// Iterate the parent with range based for loop
    using FromParents = query::From<Parent>;
    using ParentFields = query::F<Parent>;
    FromParents from_parents;
    // Select the query which you want to execute
    auto parents_where = from_parents.where(ParentFields::f2 == "Delete Me");
    // Fetch all the objects satisfying the query. This is a lazy fetch. It will be fetched
    // only when it is called. And not all the objects are fetched.
�   // Here v is a PRef so it can be used to modify and persist the object.
    for(auto v : parents_where) {
        std::cout << v.toJson() << std::endl;
    }
```

## Object conversion to JSON & From JSON and Object conversion to Message Pack & from Message Pack
With Bun you can do the following too:
* Convert an C++ object to JSON
* Create an C++ object from JSON
* Convert and C++ object to Message Pack
* Create an C++ object from Message Pack
The following code demonstrates that:
```C++
#include "blib/bun/bun.hpp"

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
    return 1;
}
```

## Key value store
Bun has a key value store. Currently the Keyvalue store supported is Unqlite. Following code explains how to use it:

```C++
#include "blib/bun/bun.hpp"

/// @fn kvTest
/// @brief A test program for 
int kvTest() {
    /// @var db
    /// @brief Create the database. If the database already exists 
    /// it opens the database but creates if it doesnt exist
    blib::bun::KVDb<> db("kv.db");
    /// @brief put a value in database.
    db.put("test", "test");
    std::string val;
    /// @brief get the value. We need to pass a variable by reference to get the value.
    db.get("test", val);
    std::cout << val << std::endl;
    
    const int size = 10000;
    for (int i = 0; i < size; ++i) {
        const std::string s = fmt::format("Value: {}", i);
        db.put(i, s);
    }

    for (int i = 0; i < size; ++i) {
        std::string val;
        db.get(i, val);
        std::cout << val << std::endl;
    }
    
    return 1;
}
```
# Help Wanted
Considering the work needed to make this library further enrich I will be needing any help needed.
Help is needed in the following areas.

1. Enhancement
2. Fix bugs.
3. Restructure and cleanup code.
4. Enhance documentation.
5. Constructive criticism and feature suggestions.
6. Write tests.
7. Use Bun

# History
### Alpha 1 (16th May 2016)
* Initial version of the library
### Alpha 2 (2nd July 2016)
Implementing the Bun EDSL
### Alpha 3 (14th March 2018):
* Integrated SOCI as the database interaction layer. This makes the library use any SQL database as SQLite, Postgres, MySQL. It mostly supports other databases that SOCI supports but it's not tested yet.
* Use of Boost Fusion. The code is much cleaner, fewer preprocessor macros The code is more debuggable.
* Support for transaction handling using the Transaction class
* Better error handling and error logging
* Added a lot of comments to help users
### Alpha 4 (5th March 2018)
* Support for nested objects
* SimpleOID now uses boost UUID to generate a unique identifier
* Additional comments
* Small performance enhancements
### Alpha 5 (19th May 2018)
* Support for constraint before table creation
### Alpha 6 (18th July 2018):
* Adding key value functionality to bun
### Alpha 7 (11 August 2018):
* Added range based for loop support for object iteration.
* Added range based for loop support for key-value store iteration.
* Both the iterations are lazy iterations.
### Alpha 8 (19 October 2018)
* Added support to create C++ object from JSON string
* Added support to create Message Pack from C++ object
* Added support to create C++ object from Message Pack