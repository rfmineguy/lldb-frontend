#ifndef UTIL_HPP
#define UTIL_HPP
#include <lldb/API/LLDB.h>
#include <filesystem>
#include <optional>

struct Line;
namespace Util {
  enum class SystemTheme {
    DARK, LIGHT,
  };

  std::string SystemThemeToString(SystemTheme theme);

  void PrintTargetModules(lldb::SBTarget& target);
  void PrintModuleCompileUnits(lldb::SBTarget& target, int moduleIdx);
  std::filesystem::path GetCurrentProgramDirectory();
  std::string StringEscapeBackslash(const std::string& string);
  std::optional<std::filesystem::path> GetTargetSourceRootDirectory(std::filesystem::path start_dir);

  bool ReadFileLinesIntoVector(const std::filesystem::path& filepath, std::vector<Line>& lines);
  inline static std::string PathSeparator = std::string(1, char(std::filesystem::path::preferred_separator));

  SystemTheme GetSystemTheme();

  std::vector<std::string> ConvertArgsToArgv(std::vector<std::string>& args);
}

#endif
