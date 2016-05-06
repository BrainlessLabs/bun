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
  blib::bun::PRef<test::Test> t = new test::Test;
  blib::bun::PRef<test::Test> t1 = new test::Test;
  return 0;
}

