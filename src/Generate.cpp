#include "blib/bun/Bun.hpp"
#include <string>

namespace test {
  struct Person {
    std::string name;
    int age;
    float height;
  };
}

GENERATE_BINDING( (test::Person, name, age, height) );

