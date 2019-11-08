#include <boost/proto/proto.hpp>
#include <boost/mpl/vector.hpp>
#include <type_traits>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
#include "blib/bun/bun.hpp"

// BOOST_PROTO_MAX_LOGICAL_ARITY
using namespace boost;
namespace test {
  struct Person {
    std::string name;
    int age;
    float height;
  };
}

int main1() {
  auto p = proto::as_expr( std::string( "10" ) );
  return 0;
}
