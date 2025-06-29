#ifndef LLDB_DEBUGGER_HPP
#define LLDB_DEBUGGER_HPP
#include <lldb/API/LLDB.h>
#include <string>

struct FileContext;

class LLDBDebugger {
  public:
    enum class ExecResult {
      Ok,
    };
  public:
    LLDBDebugger();
    ~LLDBDebugger();

    lldb::SBDebugger& GetDebugger(); 
    lldb::SBTarget GetTarget();
    void SetTarget(lldb::SBTarget target);

    bool AddBreakpoint(FileContext& fctx, int id);
    bool RemoveBreakpoint(FileContext& fctx, int id);

  public:
    ExecResult ExecCommand(const std::string&);

  private:
    lldb::SBDebugger debugger;
};

#endif
