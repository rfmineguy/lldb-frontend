#ifndef UTIL_HPP
#define UTIL_HPP
#include <lldb/API/LLDB.h>
#include <filesystem>
#include <optional>

namespace Util {
  void PrintTargetModules(lldb::SBTarget& target);
  void PrintModuleCompileUnits(lldb::SBTarget& target, int moduleIdx);
  std::filesystem::path GetCurrentProgramDirectory();
  std::string StringEscapeBackslash(const std::string& string);
  std::optional<std::filesystem::path> GetTargetSourceRootDirectory(std::filesystem::path start_dir);
}

#endif
