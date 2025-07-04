#ifndef LLDB_DEBUGGER_HPP
#define LLDB_DEBUGGER_HPP
#include <lldb/API/LLDB.h>
#include <string>
#include "FileHierarchy.hpp"
#include <thread>

struct FileContext;
#include <thread>
#include <fmt/core.h>
#include "LLDBCommandParser.hpp"
#include "TempRedirect.hpp"

class ImGuiLayer;

class LLDBDebugger {
  friend Window;
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

    bool AddBreakpoint(FileHierarchy::TreeNode&, int id);
    bool RemoveBreakpoint(FileHierarchy::TreeNode&, int id);
  private:
    void HitBreakpoint(lldb::break_id_t b_id);
    void SetActiveLine(BreakpointData bdata);
    bool CanRunCommand();
  public:
    BreakpointData& GetBreakpointData(lldb::break_id_t id);
    bool IsActiveFile(const std::string& filename);
    std::string GetActiveFile() const;
    bool IsActiveLine(int line_number);

  public:
    ExecResult ExecCommand(const std::string&, FileHierarchy&);

  public:
    void Continue();
    void StepInto();
    void StepOver();
    void Next();

  private:
    void LLDBEventThread();

  private:
    lldb::SBDebugger debugger;
    std::unordered_map<lldb::break_id_t, BreakpointData> id_breakpoint_data;
    std::optional<BreakpointData> active_line;

    lldb::SBTarget target;
    lldb::SBProcess process;
    lldb::SBListener listener;
    std::thread lldbEventThread;
    TempRedirect in_redirect;
    size_t in_offset = 0;
    TempRedirect out_redirect;
    size_t out_offset = 0;
    TempRedirect err_redirect;
    size_t err_offset = 0;
    void DumpToStd(TempRedirect &redirect, std::ostream &out, size_t& offset);

  protected:
    ImGuiLayer* imGuiLayer_ptr = 0;

  private:
    LLDB_CommandParser commandParser;
};

#endif
