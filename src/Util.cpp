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
}
