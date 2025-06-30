#include "Util.hpp"
#include "Logger.hpp"
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace Util {
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
}