#include "LLDBDebugger.hpp"
#include "FileContext.hpp"
#include <filesystem>
#include <stdexcept>
#include "Logger.hpp"
#include <unistd.h>

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
  if (!GetTarget().IsValid()) {
    Logger::Crit("Failed to launch target. Target not valid. {}", target.GetExecutable().GetFilename());
    return;
  }

  // Setup launch info
  lldb::SBLaunchInfo launch_info(nullptr);
  launch_info.AddDuplicateFileAction(STDOUT_FILENO, STDOUT_FILENO);
  launch_info.AddDuplicateFileAction(STDERR_FILENO, STDERR_FILENO);
  launch_info.AddDuplicateFileAction(STDIN_FILENO, STDIN_FILENO);

  lldb::SBError error;
  lldb::SBProcess process = GetTarget().Launch(launch_info, error);
  Logger::Info("Is target valid? {}", target.IsValid() ? "Yes" : "No");
  Logger::Info("Launch Status: {}", error.Success() ? "Success" : "Fail");
  Logger::Info("Is process valid? {}", process.IsValid() ? "Yes" : "No");

  // If the event thread has been run before, join it so we can spawn it again
  if (lldbEventThread.joinable()) lldbEventThread.join();
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

void LLDBDebugger::SetTarget(lldb::SBTarget target) {
  debugger.SetSelectedTarget(target);
}

bool LLDBDebugger::AddBreakpoint(FileContext& fctx, int id) {
    if (id < 0 || id >= static_cast<int>(fctx.lines.size())) {
        return false;
    }

    Line& line = fctx.lines[id];
    if (line.bp) {
        return false;
    }

    auto target = GetTarget();

    const char* filename = fctx.filename.c_str();
    int line_number = id + 1;
    lldb::SBBreakpoint bp = target.BreakpointCreateByLocation(filename, line_number);

    if (bp.IsValid()) {
        line.bp = true;
        line.bp_id = bp.GetID();
        auto real_filename = std::filesystem::path(fctx.filename).filename().string();
        id_breakpoint_data[line.bp_id] = {real_filename, line_number};
        return true;
    }

    return false;
}

bool LLDBDebugger::RemoveBreakpoint(FileContext& fctx, int id) {
    if (id < 0 || id >= static_cast<int>(fctx.lines.size())) {
        return false;
    }

    Line& line = fctx.lines[id];
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
  while (running && listener.WaitForEvent(1, event)) {
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
  Logger::Info("LLDB Event Thread Stopping");
}
