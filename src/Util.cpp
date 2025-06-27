#include "Util.hpp"
#include <iostream>

namespace Util {
  void PrintTargetModules(lldb::SBTarget& target) {
    std::cout << "TargetModules:" << std::endl;
    for (int i = 0; i < target.GetNumModules(); i++) {
      auto module = target.GetModuleAtIndex(i);
      std::cout << "\t" << i << " : " << module.GetFileSpec().GetFilename() << std::endl;
    }
  }

  void PrintModuleCompileUnits(lldb::SBTarget& target, int moduleIdx) {
    uint32_t compileUnitCount = target.GetModuleAtIndex(0).GetNumCompileUnits();
    std::cout << "Compile Unit Count: " << compileUnitCount << std::endl;

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

        std::cout << "FileSpec Path: " << path << std::endl;
      }
    }
  }
}
