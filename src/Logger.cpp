#include "Logger.hpp"
#include <assert.h>

int Logger::log_depth = 0;
std::stack<std::string> Logger::groupStack = {};
std::stack<std::string> Logger::lineStack = {};

Logger::ScopedGroup::ScopedGroup(const std::string &tag) {
  BeginGroup(tag);
}
Logger::ScopedGroup::~ScopedGroup() {
  EndGroup();
}


void Logger::BeginGroup(const std::string & tag) {
  Println("{}[{}]\n", std::string(log_depth * LOG_SPACING, ' '), tag);
  groupStack.push(tag);
  log_depth++;
}
void Logger::EndGroup() {
  log_depth--;
  Println("{}[{}]\n", std::string(log_depth * LOG_SPACING, ' '), groupStack.top());
  groupStack.pop();
}
void Logger::BeginLine(const std::string& tag) {
  assert(0 && "Unimplemented");
}
void Logger::EndLine() {
  assert(0 && "Unimplemented");
}
