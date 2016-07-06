# bun
Bun is a simple to use C++ Object Relational Mapper (ORM) library
<p>
[Bun Page](http://brainlesslabs.github.io/bun/)
<p>
[CodeProject Article](http://www.codeproject.com/Tips/1100449/Cplusplus-Object-Relational-Mapping-ORM-Eating-the)
<p>
[![OpenHub](https://www.openhub.net/p/brainlesslabs_bun/widgets/project_thin_badge.gif)](https://www.openhub.net/p/brainlesslabs_bun)
#Usage

```C++
// Bun.cpp : Defines the entry point for the console application.
//

#include "blib/bun/SqliteDb.hpp"

// Class to persist
namespace test {
  struct Person {
    std::string name;
    int age;
    float height;
  };
}
// Generate the binding for the db. Safe to have in different headers or cpp files.
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
  for (int i = 1; i < 10; ++i) {
    // PRef is a reference to the persistant object.
    // PRef keeps the ownership of the memory. Release the memory when it is destroyed.
    // Internally it holds the object in a unique_ptr
    // PRef also has a oid associated with the object
    bun::PRef<test::Person> p = new test::Person;
    // Assign the members values
    p->age = i+10;
    p->height = 5.6;
    p->name = fmt::format("Brainless_{}", i);
    // Persist the object and get a oid for the persisted object.
    const blib::bun::SimpleOID oid = p.persist();

    //Getting the object from db using oid.
    bun::PRef<test::Person> p1(oid);
  }

  // To get all the object oids of a particular object.
  // oids is a vector of tyoe std::vector<blib::bun<>SimpleOID<test::Person>>
  const auto person_oids = blib::bun::getAllOids<test::Person>();

  // To get the objects of a particular type
  // std::vector<blib::bun::Pref<test::Person>>
  const auto person_objs = blib::bun::getAllObjects<test::Person>();

  return 0;
}
```
