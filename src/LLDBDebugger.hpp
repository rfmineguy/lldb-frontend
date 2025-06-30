#ifndef LLDB_DEBUGGER_HPP
#define LLDB_DEBUGGER_HPP
#include <lldb/API/LLDB.h>
#include <string>
#include "FileHeirarchy.hpp"
#include <thread>

struct FileContext;
#include <thread>
#include <fmt/core.h>
#include "LLDBCommandParser.hpp"

class LLDBDebugger {
  public:
    enum class ExecResultStatus {
      Ok,
      CommandDoesNotExist,
      HandleResolveFail,
    };
    struct ExecResult {
      ExecResultStatus status;
      std::string message;

      template <typename ...Args>
      static ExecResult Err(ExecResultStatus status, std::string_view fmt, Args&&... args) { 
        return ExecResult{
          .status = status,
          .message = fmt::vformat(fmt, fmt::make_format_args(args...))
        };
      }
      static ExecResult Ok() { return ExecResult{.status =  ExecResultStatus::Ok}; }
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
    lldb::SBProcess GetProcess();
    void SetTarget(lldb::SBTarget target);

    bool AddBreakpoint(FileHeirarchy::HeirarchyElement&, int id);
    bool RemoveBreakpoint(FileHeirarchy::HeirarchyElement&, int id);
    BreakpointData& GetBreakpointData(lldb::break_id_t id);

  public:
    ExecResult ExecCommand(const std::string&, FileHeirarchy&);

  private:
    void LLDBEventThread();

  private:
    lldb::SBDebugger debugger;
    std::unordered_map<lldb::break_id_t, BreakpointData> id_breakpoint_data;

    lldb::SBTarget target;
    lldb::SBProcess process;
    lldb::SBListener listener;
    std::thread lldbEventThread;

  private:
    LLDB_CommandParser commandParser;
};

#endif
