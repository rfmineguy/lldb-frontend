#include "nested.hpp"
#include <iostream>

Nested::Nested() {
  std::cout << "Nested constructed" << std::endl;
}

Nested::~Nested() {
  std::cout << "Nested destructed" << std::endl;
}
