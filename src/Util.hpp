#ifndef UTIL_HPP
#define UTIL_HPP
#include <lldb/API/LLDB.h>

namespace Util {
  void PrintTargetModules(lldb::SBTarget& target);
  void PrintModuleCompileUnits(lldb::SBTarget& target, int moduleIdx);
}

#endif
