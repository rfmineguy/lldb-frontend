#ifndef UTIL_HPP
#define UTIL_HPP
#include <lldb/API/LLDB.h>
#include <filesystem>
#include <optional>

struct Line;
namespace Util {
  void PrintTargetModules(lldb::SBTarget& target);
  void PrintModuleCompileUnits(lldb::SBTarget& target, int moduleIdx);
  std::filesystem::path GetCurrentProgramDirectory();
  std::string StringEscapeBackslash(const std::string& string);
  std::optional<std::filesystem::path> GetTargetSourceRootDirectory(std::filesystem::path start_dir);

  bool ReadFileLinesIntoVector(const std::string& filepath, std::vector<Line>& lines);
}

#endif
