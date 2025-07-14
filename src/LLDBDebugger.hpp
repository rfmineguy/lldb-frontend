#ifndef LLDB_DEBUGGER_HPP
#define LLDB_DEBUGGER_HPP
#include <lldb/API/LLDB.h>
#include <string>
#include "FileHierarchy.hpp"
#include <thread>

struct FileContext;
#include <thread>
#include <variant>
#include <fmt/core.h>
#include "LLDBCommandParser.hpp"
#include "TempRedirect.hpp"

class LLDBDebugger {
  friend class Window;
  public:
    struct Event {
      struct Continue {};
      struct StepOver {};
      struct StepInto {};
      struct LoadFile {
        FileHierarchy::TreeNode* node;
      };
      struct IO {
        std::string data;
      };
      struct SwitchToFile {
        std::string filename;
      };
      std::variant<Continue, StepOver, StepInto, LoadFile, IO, SwitchToFile> data;
    };
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
      std::filesystem::path path;
      // std::string filename;
      int line_number;
    };
  public:
    LLDBDebugger();
    ~LLDBDebugger();
    void SetEventCallback(std::function<void(const Event&)> eventCallback);

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
    std::filesystem::path GetActiveFile() const;
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
    std::function<void(const Event&)> eventCallback;

  private:
    LLDB_CommandParser commandParser;
};

#endif
