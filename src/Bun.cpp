// Bun.cpp : Defines the entry point for the console application.
//

#include "blib/bun/bun.hpp"

namespace test {
  // Class that needs to be persisted
  struct Person {
    std::string name;
    int age;
    float height;
  };
}

/////////////////////////////////////////////////
/// Generate the database bindings at compile time.
/////////////////////////////////////////////////
SPECIALIZE_BUN_HELPER( (test::Person, name, age, height) );

int main() {
  namespace bun = blib::bun;
  namespace query = blib::bun::query;

  // Connect the db. If the db is not there it will be created.
  // It should include the whole path
  bun::connect( "objects.db" );
  // Create the schema. We can create the schema multile times. If its already created
  // it will be safely ignored
  bun::createSchema<test::Person>();

  // Create some entries in the database
 bun::Transaction t;
  for (int i = 1; i < 10000; ++i) {
    // PRef is a reference to the persistant object.
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
  using PersonFields = query::F<test::Person>;
  using FromPerson = query::From<test::Person>;
  FromPerson fromPerson;
  // Grammar are checked for validity of syntax at compile time itself.
  // Currently only &&, ||, <, <=, >, >=, ==, != are supported. They have their respective meaning
  // Below is a valid query grammar
  auto valid_query = PersonFields::age > 10 && PersonFields::name != "Brainless_0";
  std::cout << "Valid Grammar?: " << query::IsValidQuery<decltype(valid_query)>::value << std::endl;

  // Oops + is not a valid grammar
  auto invalid_query = PersonFields::age + 10 && PersonFields::name != "Brainless_0";
  std::cout << "Valid Grammar?: " << query::IsValidQuery<decltype(invalid_query)>::value << std::endl;

  // Now let us execute the query.
  // The where function also checks for the validity of the query, and fails at compile time
  const auto objs = fromPerson.where( valid_query ).where( valid_query ).objects();
  // Can even use following way of query
  // As you see we can join queries 
  const auto q = PersonFields::age > 21 && PersonFields::name == "test";
  const auto objs_again = FromPerson().where( q ).objects();
  const auto objs_again_q = FromPerson().where( PersonFields::age > 21 && PersonFields::name == "test" ).objects();
  // Not going to compile if you enable the below line. Will get the "Syntax error in Bun Query" compile time message.
  //const auto objs1 = FromPerson.where( invalid_query ).objects();

  // Check the query generated. It does not give the sql query.
  std::cout << fromPerson.query() << std::endl;

  return 0;
}

