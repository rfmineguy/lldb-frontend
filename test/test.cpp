#include <iostream>
#include "nested/nested.hpp"

void testFunction() {
  std::cout << "testFunction()" << std::endl;
}

int main(int argc, const char **argv) {
  std::cout << "argv[0]: " << argv[0] << std::endl;
  std::cout << "Hello world from test" << std::endl;
  std::cout << "Hello world from test" << std::endl;
  std::cout << "Hello world from test" << std::endl;

  int x = 4;
  while (x > 0) {
    std::cout << "x: " << x << std::endl;
    x--;
  }

  Nested n;
}
