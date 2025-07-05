#include "nested.hpp"
#include <iostream>

Nested::Nested()
:value(5), s("affsd") {
  std::cout << "Nested constructed" << std::endl;
}

Nested::Nested(int value, const std::string& s)
  :value(value), s(s) {
  std::cout << "Nested constructed" << std::endl;
}

Nested::~Nested() {
  std::cout << "Nested destructed" << std::endl;
}
