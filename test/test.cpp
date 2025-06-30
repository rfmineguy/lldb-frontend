#include <iostream>

void testFunction() {
  std::cout << "testFunction()" << std::endl;
}

int main() {
  std::cout << "Hello world from test" << std::endl;
  std::cout << "Hello world from test" << std::endl;
  std::cout << "Hello world from test" << std::endl;

  int x = 4;
  while (x > 0) {
    std::cout << "x: " << x << std::endl;
    x--;
  }
}
