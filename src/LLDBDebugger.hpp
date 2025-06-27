#ifndef LLDB_DEBUGGER_HPP
#define LLDB_DEBUGGER_HPP
#include <lldb/API/LLDB.h>

class LLDBDebugger {
  public:
    LLDBDebugger();
    ~LLDBDebugger();

    lldb::SBDebugger& GetDebugger(); 
    lldb::SBTarget& GetTarget();
  private:
    lldb::SBDebugger debugger;
    lldb::SBTarget target;
};

#endif
