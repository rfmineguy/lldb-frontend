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
}
