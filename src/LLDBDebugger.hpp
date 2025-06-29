#ifndef LLDB_DEBUGGER_HPP
#define LLDB_DEBUGGER_HPP
#include <lldb/API/LLDB.h>
#include <string>
#include "FileHeirarchy.hpp"
#include <thread>

struct FileContext;

class LLDBDebugger {
  public:
    enum class ExecResult {
      Ok,
    };
    struct BreakpointData
    {
      std::string filename;
      int line_number;
    };
  public:
    LLDBDebugger();
    ~LLDBDebugger();

    void LaunchTarget();
    lldb::SBDebugger& GetDebugger(); 
    lldb::SBTarget GetTarget();
    void SetTarget(lldb::SBTarget target);

    bool AddBreakpoint(FileHeirarchy::HeirarchyElement&, int id);
    bool RemoveBreakpoint(FileHeirarchy::HeirarchyElement&, int id);
    BreakpointData& GetBreakpointData(lldb::break_id_t id);

  public:
    ExecResult ExecCommand(const std::string&);

  private:
    void LLDBEventThread();

  private:
    lldb::SBDebugger debugger;
    std::unordered_map<lldb::break_id_t, BreakpointData> id_breakpoint_data;

    lldb::SBTarget target;
    lldb::SBListener listener;
    std::thread lldbEventThread;
};

#endif
