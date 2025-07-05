#ifndef NESTED_HPP
#define NESTED_HPP
#include <string>

class Nested {
  public:
    Nested();
    Nested(int value, const std::string& s);
    ~Nested();
  private:
    int value;
    std::string s;
};

#endif
