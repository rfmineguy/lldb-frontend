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
