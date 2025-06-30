#include "LLDBDebugger.hpp"
#include "FileContext.hpp"
#include <filesystem>
#include <stdexcept>
#include "Logger.hpp"
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#include <windows.h>
#endif

LLDBDebugger::LLDBDebugger() {
    lldb::SBDebugger::Initialize();
    debugger = lldb::SBDebugger::Create();
    debugger.SetAsync(true);
    listener = debugger.GetListener();
}

LLDBDebugger::~LLDBDebugger() {
  target.GetProcess().Kill();           // TODO: We have a problem where the running thread keeps the main process from closing
  lldb::SBDebugger::Destroy(debugger);
  lldb::SBDebugger::Terminate();
  if (lldbEventThread.joinable())
    lldbEventThread.join();
}

void LLDBDebugger::LaunchTarget() {
  Logger::ScopedGroup g("LaunchTarget");
  auto target = GetTarget();
  if (!target.IsValid()) {
    Logger::Crit("Failed to launch target. Target not valid.");
    return;
  }

  auto exe_spec = target.GetExecutable();
  char exe_path[1024] = {};
  uint32_t path_len = exe_spec.GetPath(exe_path, sizeof(exe_path));
  if (path_len == 0) {
    Logger::Err("Failed to get executable path");
    return;
  }
  auto exe_path_string = std::string(exe_path);
  Logger::Info("Executable path: {}", exe_path_string.c_str());

  if (!std::filesystem::exists(exe_path_string.c_str())) {
    Logger::Err("Executable path does not exist on disk");
    return;
  }

  auto workdir = exe_spec.GetDirectory();
  if (!workdir) {
    Logger::Err("Failed to get executable directory");
    return;
  }
  Logger::Info("Working directory: {}", workdir);

  lldb::SBError error;
  auto exe_path_string_esc = Util::StringEscapeBackslash(exe_path_string);
  process = target.Launch(
    listener,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    workdir,
    0,
    true,
    error
  );

  if (!target.IsValid()) {
    Logger::Crit("Target not valid.");
    return;
  }
  Logger::Info("Launch Status: {}", error.Success() ? "Success" : "Fail");
  Logger::Info("Launch Error Message: {}", error.GetCString());
  Logger::Info("Launch error code: 0x{:X}", error.GetError());
  Logger::Info("Is process valid? {}", process.IsValid() ? "Yes" : "No");

  if (error.Fail()) {
    Logger::Err("Aborting event thread: launch failed");
    return;
  }

  if (lldbEventThread.joinable())
    lldbEventThread.join();
  lldbEventThread = std::thread([&]() {
    LLDBEventThread();
  });

  Logger::Info("Launched target");
}

lldb::SBDebugger& LLDBDebugger::GetDebugger() {
  return debugger;
}

lldb::SBTarget LLDBDebugger::GetTarget() {
  return debugger.GetSelectedTarget();
}

lldb::SBProcess LLDBDebugger::GetProcess() {
  return process;
}

void LLDBDebugger::SetTarget(lldb::SBTarget target) {
  debugger.SetSelectedTarget(target);
}

bool LLDBDebugger::AddBreakpoint(FileHeirarchy::HeirarchyElement& element, int id) {
  if (element.lines->empty()) return false;

    if (id < 0 || id >= static_cast<int>(element.lines->size())) {
        return false;
    }

    Line& line = element.lines->at(id);
    if (line.bp) {
        return false;
    }

    auto target = GetTarget();

    const char* filename = element.c_str;
    int line_number = id + 1;
    lldb::SBBreakpoint bp = target.BreakpointCreateByLocation(filename, line_number);

    if (bp.IsValid()) {
        line.bp = true;
        line.bp_id = bp.GetID();
        auto real_filename = element.full_path.string();
        id_breakpoint_data[line.bp_id] = {real_filename, line_number};
        return true;
    }

    return false;
}

bool LLDBDebugger::RemoveBreakpoint(FileHeirarchy::HeirarchyElement& element, int id) {
  if (element.lines->empty()) return false;
    if (id < 0 || id >= static_cast<int>(element.lines->size())) {
        return false;
    }

    Line& line = element.lines->at(id);
    if (!line.bp || line.bp_id == LLDB_INVALID_BREAK_ID) {
        return false;
    }

    auto target = GetTarget();
    if (target.BreakpointDelete(line.bp_id)) {
        line.bp = false;
        id_breakpoint_data.erase(line.bp_id);
        line.bp_id = LLDB_INVALID_BREAK_ID;
        return true;
    }

    return false;
}

LLDBDebugger::BreakpointData& LLDBDebugger::GetBreakpointData(lldb::break_id_t id)
{
  auto it = id_breakpoint_data.find(id);
  if (it == id_breakpoint_data.end())
    throw std::runtime_error("Could not find breakpoint with id");
  return it->second;
}

LLDBDebugger::ExecResult LLDBDebugger::ExecCommand(const std::string& command) {
  return ExecResult::Ok;
}

void LLDBDebugger::LLDBEventThread() {
  using namespace lldb;
  SBEvent event;
  bool running = true;
  while (running) {
    if (listener.WaitForEvent(1, event)) {
      if (SBProcess::EventIsProcessEvent(event)) {
        Logger::Info("Event name: {}", event.GetBroadcaster().GetName());
        StateType state = SBProcess::GetStateFromEvent(event);
        switch (state) {
          case eStateStopped:
            Logger::Info("Target stopped");
            break;
          case eStateExited:
            Logger::Info("Target exited");
            running = false;
            break;
          case eStateRunning:
            Logger::Info("Target running");
            break;
          case eStateCrashed:
            Logger::Info("Target crashed");
            break;
          case eStateInvalid:
            Logger::Info("Invaid State");
            break;
          case eStateAttaching:
            Logger::Info("Target attaching");
            break;
          case eStateConnected:
            Logger::Info("Target connected");
            break;
          case eStateDetached:
            Logger::Info("Target detatched");
            break;
          case eStateLaunching:
            Logger::Info("Target launching");
            break;
          case eStateStepping:
            Logger::Info("Target stepping");
            break;
          case eStateSuspended:
            Logger::Info("Target suspended");
            break;
          case eStateUnloaded:
            Logger::Info("Target unloaded");
            break;
          default:
            break;
        }
      }
    }
  }
  Logger::Info("LLDB Event Thread Stopping");
}
