#include "Util.hpp"
#include "Logger.hpp"
#include <iostream>

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
}
