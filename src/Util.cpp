#include "Util.hpp"
#include "FileContext.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

struct Line;
namespace Util {
  std::string SystemThemeToString(SystemTheme theme) {
    switch (theme) {
      case Util::SystemTheme::DARK: return "Dark";
      case Util::SystemTheme::LIGHT: return "Light";
    }
  }

  void PrintTargetModules(lldb::SBTarget& target) {
    Logger::ScopedGroup g("PrintTargetModules");
    for (int i = 0; i < target.GetNumModules(); i++) {
      auto module = target.GetModuleAtIndex(i);
      Logger::Info("{} : {}", i, module.GetFileSpec().GetFilename());
    }
  }

  void PrintModuleCompileUnits(lldb::SBTarget& target, int moduleIdx) {
    Logger::ScopedGroup g("PrintModuleCompileUnits");
    uint32_t compileUnitCount = target.GetModuleAtIndex(0).GetNumCompileUnits();
    Logger::Info("Compile Unit Count: {}", compileUnitCount);

    for (size_t i = 0; i < target.GetNumModules(); i++) {
      auto module = target.GetModuleAtIndex(i);
      if (!module.IsValid()) continue;

      for (size_t j = 0; j < module.GetNumCompileUnits(); j++) {
        auto compileUnit = module
          .GetCompileUnitAtIndex(j);

        lldb::SBFileSpec fs = compileUnit.GetFileSpec();
        if (!fs.IsValid()) continue;
        const char* path = fs.GetFilename();
        if (path == NULL) continue;

        Logger::Info("FileSpec Path: {}", path);
      }
    }
  }

  std::filesystem::path GetCurrentProgramDirectory() {
    std::filesystem::path exePath;

#if defined(_WIN32)
    char path[MAX_PATH];
    DWORD length = GetModuleFileNameA(NULL, path, MAX_PATH);
    if (length > 0 && length < MAX_PATH) {
        exePath = std::filesystem::canonical(path);
    }

#elif defined(__APPLE__)
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        exePath = std::filesystem::canonical(path);
    }

#elif defined(__linux__)
    exePath = std::filesystem::canonical("/proc/self/exe");

#endif

    return exePath.parent_path();
  }
  
  std::string StringEscapeBackslash(const std::string& string)
  {
    std::string result;

    // Reserve enough space: worst case all backslashes need escaping
    result.reserve(string.size() * 2);

    std::size_t i = 0;
    const std::size_t length = string.size();

    while (i < length)
    {
        if (string[i] == '\\')
        {
            // Count consecutive backslashes starting at i
            std::size_t backslash_count = 0;

            while (i + backslash_count < length && string[i + backslash_count] == '\\')
            {
                ++backslash_count;
            }

            // Number of full pairs
            std::size_t pairs = backslash_count / 2;

            // Copy all full pairs as-is (each pair is two backslashes)
            for (std::size_t p = 0; p < pairs; ++p)
            {
                result.push_back('\\');
                result.push_back('\\');
            }

            // If there is an odd one left (unescaped), escape it
            if (backslash_count % 2 != 0)
            {
                // One unescaped backslash, so write two backslashes
                result.push_back('\\');
                result.push_back('\\');
            }

            // Move i forward by backslash_count
            i += backslash_count;
        }
        else
        {
            // Normal character, just copy
            result.push_back(string[i]);
            ++i;
        }
    }

    return result;
  }

  static std::vector<std::string> projectRootMarkers = {
    "CMakeLists.txt",
    "Makefile",
    "build.zig",
    "meson.build",
    "Cargo.toml",
    "package.json",
    "pyproject.toml",
    ".git"
  };

  bool isRoot(const std::filesystem::path& current) {
    std::filesystem::path normalized = current.lexically_normal();
    return normalized == normalized.root_path();
  }

  std::optional<std::filesystem::path> GetTargetSourceRootDirectory(std::filesystem::path start_dir) {
    using namespace std::filesystem;
    path current = start_dir.parent_path();
    bool at_root = false;
    while (!current.empty()) {
      Logger::Info("Current: {}", current.string());
      for (const auto& marker : projectRootMarkers) {
        if (std::filesystem::exists(current / marker)) {
          Logger::Info("Has marker '{}': {}", marker, current.string());
          return current;
        }
      }
      if (at_root)
        break;
      current = current.parent_path();
      if (isRoot(current))
        at_root = true;
    }
    return std::nullopt;
  }

  bool ReadFileLinesIntoVector(const std::filesystem::path& filepath, std::vector<Line>& lines) {
    std::ifstream f(filepath);
    if (!f.is_open()) {
      Logger::Crit("Failed to load {} from disk", filepath.string());
      f.close();
      return false;
    }
    std::string line;
    while (std::getline(f, line)) {
      lines.push_back(Line{.line = line, .bp = false});
    }
    f.close();
    return true;
  }

  SystemTheme GetSystemTheme() {
#if defined(_WIN32)
    // Windows 10/11 via registry
    HKEY hKey;
    DWORD value = 1;
    DWORD dataSize = sizeof(value);
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
                      "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExA(hKey, "AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&value, &dataSize) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return value == 0 ? SystemTheme::DARK : SystemTheme::LIGHT; // 0 = dark, 1 = light
        }
        RegCloseKey(hKey);
    }
    return SystemTheme::LIGHT;

#elif defined(__APPLE__)
    // macOS via Apple Interface Style
    FILE* pipe = popen("defaults read -g AppleInterfaceStyle 2>/dev/null", "r");
    if (!pipe)
        return SystemTheme::LIGHT;
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }
    pclose(pipe);
    for (auto& c : result)
        c = std::tolower(c);
    Logger::Info("Result: {}", result);
    return (result.find("dark") != std::string::npos) ? SystemTheme::DARK : SystemTheme::LIGHT;

#elif defined(__linux__)
    // Linux: try GSettings (GNOME-based)
    FILE* pipe = popen("gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null", "r");
    if (!pipe)
        return SystemTheme::LIGHT;
    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result += buffer;
    }
    pclose(pipe);
    for (auto& c : result)
        c = std::tolower(c);
    return (result.find("dark") != std::string::npos) ? SystemTheme::DARK : SystemTheme::LIGHT;

#else
    return SystemTheme::LIGHT; // unsupported platform
#endif
  }

  std::vector<std::string> ConvertArgsToArgv(std::vector<std::string>& args) {
    std::vector<std::string> converted;
    for (auto& arg : args) {
      size_t loc = arg.find(' ');
      if (loc != std::string::npos) {
        converted.push_back(arg.substr(0, loc));
        converted.push_back(arg.substr(loc + 1));
      }
    }
    return converted;
  }
}
