// Bun.cpp : Defines the entry point for the console application.
//

#include "blib/bun/SqliteDb.hpp"

namespace test {
  struct Test {
    int one;
    float two;
    std::string three;
  };
}

GENERATE_BINDING( (test::Test, one, two) );

int main() {
  namespace bun = blib::bun;
  bun::dbConnect("test.db");
  bun::PRef<test::Test> t = new test::Test;
  bun::PRef<test::Test> t1 = new test::Test;
  t.persist();
  return 0;
}

