#ifndef LOGGER_HPP
#define LOGGER_HPP
#include <string>
#include <stack>
#include <iostream>
#include <fmt/core.h>

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

  public:
    template<typename... Args>
    constexpr static void PrintlnLevel(const char* level, std::string_view fmt, Args&&... args)
    {
      std::string formatted = fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...));
      fmt::print("{}[{}] {}", std::string(log_depth * LOG_SPACING, ' '), level, formatted);
      std::cout << std::endl;
    }

    template<typename... Args>
    static void Println(std::string_view fmt, Args&&... args)
    {
      std::string formatted = fmt::vformat(fmt, fmt::make_format_args(args...));
      std::cout << formatted << std::endl;
    }

    template<typename... Args>
    static void Info(std::string_view fmt, Args&&... args) {
      PrintlnLevel("Info", fmt, args...);
    }

    template<typename... Args>
    static void Warn(std::string_view fmt, Args&&... args) {
      PrintlnLevel("Warn", fmt, args...);
    }

    template<typename... Args>
    static void Err (std::string_view fmt, Args&&... args) {
      PrintlnLevel("Err", fmt, args...);
    }

    template<typename... Args>
    static void Crit(std::string_view fmt, Args&&... args) {
      PrintlnLevel("Crit", fmt, args...);
    }

    template<typename... Args>
    static void Todo(std::string_view fmt, Args&&... args) {
      PrintlnLevel("Todo", fmt, args...);
    }
};

#endif
