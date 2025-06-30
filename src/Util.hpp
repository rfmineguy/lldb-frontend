#ifndef UTIL_HPP
#define UTIL_HPP
#include <lldb/API/LLDB.h>
#include <filesystem>

namespace Util {
  void PrintTargetModules(lldb::SBTarget& target);
  void PrintModuleCompileUnits(lldb::SBTarget& target, int moduleIdx);
  std::filesystem::path GetCurrentProgramDirectory();
  std::string StringEscapeBackslash(const std::string& string);
}

#endif
