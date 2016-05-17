# bun
Bun is a simple to use C++ Object Relational Mapper (ORM) library
[Bun](http://brainlesslabs.github.io/bun/)

#Usage

```C++
// Bun.cpp : Defines the entry point for the console application.
//

#include "blib/bun/SqliteDb.hpp"

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

GENERATE_BINDING( (test::Test, one, two) );
GENERATE_BINDING( (test::SomeOtherTest, one, two, three) );

int main() {
  namespace bun = blib::bun;
  bun::dbConnect( "test.db" );
  blib::bun::createSchema<test::Test>();
  blib::bun::createSchema<test::SomeOtherTest>();

  for (int i = 1; i < 100; ++i) {
    bun::PRef<test::SomeOtherTest> t = new test::SomeOtherTest;
    t->one = i;
    t->two = i * i / i + 1;
    t->three = std::to_string( t->two );
    t.persist();
  }

  for (int i = 0; i < 10; ++i) {
    bun::PRef<test::Test> t = new test::Test;
    t->one = 11;
    t->two = 666;
    t.persist();
  }

  blib::bun::SimpleOID oid;
  oid.high = 10;
  oid.low = 1547777240864505;
  bun::PRef<test::Test> t1( oid );
  t1->one = 12;
  t1->two = 667;
  t1.persist();

  auto vec_list = blib::bun::getAllOids<test::Test>();
  auto objs = blib::bun::getAllObjects<test::Test>();

  auto vec_list1 = blib::bun::getAllOids<test::SomeOtherTest>();
  auto objs1 = blib::bun::getAllObjects<test::SomeOtherTest>();

  return 0;
}
```
