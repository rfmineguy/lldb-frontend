#include <iostream>
#include <thread>
#include "nested/nested.hpp"

void testFunction() {
  std::cout << "testFunction()" << std::endl;
}

int main(int argc, const char **argv) {
  for (int i = 0; i < argc; i++) {
    std::cout << argv[i] << std::endl;
  }

  std::cout << "argv[0]: " << argv[0] << std::endl;
  std::cout << "Hello world from test" << std::endl;
  std::cout << "Hello world from test" << std::endl;
  std::cout << "Hello world from test" << std::endl;

  Nested n;
  Nested n2(9, "something else");

  std::thread thread([&] {
    std::cout << "I am in a thread!!" << std::endl; 
  });

  int x = 4;
  while (x > 0) {
    std::cout << "x: " << x << std::endl;
    x--;
  }

  if (thread.joinable()) thread.join();
}
