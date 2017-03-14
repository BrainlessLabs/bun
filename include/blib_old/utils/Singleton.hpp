#pragma once

#include <boost/core/noncopyable.hpp>

namespace blib {

  template <class T>
  class Singleton : public boost::noncopyable {
  public:
    static T& i() {
      static T _me;
      return _me;
    }
  };
}