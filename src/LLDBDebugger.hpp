#ifndef LLDB_DEBUGGER_HPP
#define LLDB_DEBUGGER_HPP
#include <lldb/API/LLDB.h>

class LLDBDebugger {
  public:
    enum class ExecResult {
      Ok,
    };
  public:
    LLDBDebugger();
    ~LLDBDebugger();

    lldb::SBDebugger& GetDebugger(); 
    lldb::SBTarget& GetTarget();

  public:
    ExecResult ExecCommand(const std::string&);

  private:
    lldb::SBDebugger debugger;
    lldb::SBTarget target;
};

#endif
