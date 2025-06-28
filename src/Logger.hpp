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

  public:
    template<typename... Args>
    static void Info(const std::format_string<Args...> fmt, Args&&... args) {
      std::string formatted = std::format(fmt, std::forward<Args>(args)...);
      std::print("{}[Info] {}\n", std::string(log_depth * LOG_SPACING, ' '), formatted);
    }

    template<typename... Args>
    static void Warn(const std::format_string<Args...> fmt, Args&&... args) {
      std::string formatted = std::format(fmt, std::forward<Args>(args)...);
      std::print("{}[Warn] {}\n", std::string(log_depth * LOG_SPACING, ' '), formatted);
    }

    template<typename... Args>
    static void Err (const std::format_string<Args...> fmt, Args&&... args) {
      std::string formatted = std::format(fmt, std::forward<Args>(args)...);
      std::print("{}[Err] {}\n", std::string(log_depth * LOG_SPACING, ' '), formatted);
    }

    template<typename... Args>
    static void Crit(const std::format_string<Args...> fmt, Args&&... args) {
      std::string formatted = std::format(fmt, std::forward<Args>(args)...);
      std::print("{}[Crit] {}\n", std::string(log_depth * LOG_SPACING, ' '), formatted);
    }

    template<typename... Args>
    static void Todo(const std::format_string<Args...> fmt, Args&&... args) {
      std::string formatted = std::format(fmt, std::forward<Args>(args)...);
      std::print("{}[Todo] {}\n", std::string(log_depth * LOG_SPACING, ' '), formatted);
    }
};

#endif
