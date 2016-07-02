// Bun.cpp : Defines the entry point for the console application.
//

#include "blib/bun/Bun.hpp"

namespace test {
  struct Test {
    int one;
    float two;
    std::string three;
  };

  struct SomeOtherTest {
    int one;
    float two;
    std::string three;
    SomeOtherTest() = default;
  };
}

namespace test {
  struct Person {
    std::string name;
    int age;
    float height;
  };
}

GENERATE_BINDING( (test::Person, name, age, height) );

int main() {
  namespace bun = blib::bun;
  // Connect the db. If the db is not there it will be created.
  // It should include the whole path
  bun::dbConnect( "test.db" );
  // Create the schema. We can create the schema multile times. If its already created
  // it will be safely ignored
  blib::bun::createSchema<test::Person>();

  // Creat some entries in the database
  for (int i = 1; i < 13; ++i) {
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
    const blib::bun::SimpleOID oid = p.persist();

    //Getting the object from db using oid.
    bun::PRef<test::Person> p1( oid );
  }

  // To get all the object oids of a particular object.
  // person_oids is a vector of type std::vector<blib::bun<>SimpleOID<test::Person>>
  const auto person_oids = blib::bun::getAllOids<test::Person>();

  // To get the objects of a particular type
  // std::vector<blib::bun::Pref<test::Person>>
  const auto person_objs = blib::bun::getAllObjects<test::Person>();

  // EDSL QUERY LANGUAGE ----------------------
  // Powerful EDSL object query syntax that is checked for syntax at compile time.
  // The compilation fails at the compile time with a message "Syntax error in Bun Query"
  bun::query::From<test::Person> FromPerson;
  // Grammar are checked for validity of syntax at compile time itself.
  // Currently only &&, ||, <, <=, >, >=, ==, != are supported. They have their respective meaning
  // Below is a valid query grammar
  auto valid_query = bun::query::F<test::Person>::age > 10 && bun::query::F<test::Person>::name != "Brainless_0";
  std::cout << "Valid Grammar?: " << blib::bun::query::IsValidQuery<decltype(valid_query)>::value << std::endl;

  // Oops + is not a valid grammar
  auto invalid_query = bun::query::F<test::Person>::age + 10 && bun::query::F<test::Person>::name != "Brainless_0";
  std::cout << "Valid Grammar?: " << blib::bun::query::IsValidQuery<decltype(invalid_query)>::value << std::endl;

  // Now let us execute the query.
  // The where function also checks for the validity of the query, and fails at compile time
  const auto objs = FromPerson.where( valid_query ).objects();

  // Not going to compile if you enable the below line. Will get the "Syntax error in Bun Query" compile time message.
  //const auto objs1 = FromPerson.where( invalid_query ).objects();

  // Check the query generated. It does not give the sql query.
  std::cout << FromPerson.query() << std::endl;
  return 0;
}
