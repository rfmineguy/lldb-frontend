#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <stack>
#include <iostream>
#include <format>

#define LOG_SPACING 4
class Logger {
  public:
    class ScopedGroup {
      public:
        ScopedGroup(const std::string&);
        ~ScopedGroup();
    };
  private:
    static int log_depth;
    static std::stack<std::string> groupStack, lineStack;

  public:
    static void BeginGroup(const std::string&);
    static void EndGroup();

  public:
    static void BeginLine(const std::string&);
    static void EndLine();
};

#endif
