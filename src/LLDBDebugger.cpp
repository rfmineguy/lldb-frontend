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
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>

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
  if (!in_redirect.Create("in") || !out_redirect.Create("out") || !err_redirect.Create("err"))
  {
      error.SetErrorString("Failed to create temporary files for I/O redirection");
      Logger::Info("Launch Error Message: {}", error.GetCString());
      return;
  }

  const char **argv = nullptr; // or fill if you need
  const char **envp = nullptr;

  auto exe_path_string_esc = Util::StringEscapeBackslash(exe_path_string);
  process = target.Launch(
    listener,
    argv,
    envp,
    in_redirect.path.c_str(),
    out_redirect.path.c_str(),
    err_redirect.path.c_str(),
    workdir,
    0,
    false,
    error
  );

  if (!target.IsValid()) {
    Logger::Crit("Target not valid.");
    return;
  }
  auto error_success = error.Success();
  Logger::Info("Launch Status: {}", error_success ? "Success" : "Fail");
  if (!error_success)
  {
    Logger::Info("Launch Error Message: {}", error.GetCString());
    Logger::Info("Launch error code: 0x{:X}", error.GetError());
  }
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

bool LLDBDebugger::AddBreakpoint(FileHierarchy::TreeNode& node, int id) {
  if (node.lines->empty()) {
    // If we try to add a breakpoint and the file hasnt been loaded yet, we have to load it
    //   so that we have access to its lines
    // There might be a better way to do this, but for now this works fine.
    node.LoadFromDisk();
  }

    if (id < 0 || id >= static_cast<int>(node.lines->size())) {
        return false;
    }

    Line& line = node.lines->at(id);
    if (line.bp) {
        return false;
    }

    auto target = GetTarget();

    const char* filename = node.name.c_str();
    int line_number = id + 1;
    lldb::SBBreakpoint bp = target.BreakpointCreateByLocation(filename, line_number);

    if (bp.IsValid()) {
        line.bp = true;
        line.bp_id = bp.GetID();
        auto& path = node.path;
        auto real_filename = path.string();
        id_breakpoint_data[line.bp_id] = {real_filename, line_number};
        Logger::Info("Set breakpoint at {} on line {}", filename, line_number);
        return true;
    }

    return false;
}

bool LLDBDebugger::RemoveBreakpoint(FileHierarchy::TreeNode& node, int id) {
  if (node.lines->empty()) return false;
    if (id < 0 || id >= static_cast<int>(node.lines->size())) {
        return false;
    }

    Line& line = node.lines->at(id);
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

void LLDBDebugger::HitBreakpoint(lldb::break_id_t b_id) {
  auto it = id_breakpoint_data.find(b_id);
  if (it == id_breakpoint_data.end())
    return;
  auto& b_data = it->second;
  active_line = b_data;
}

bool LLDBDebugger::IsActiveFile(const std::string& filename) {
  return active_line.has_value() && active_line->filename == filename;
}

bool LLDBDebugger::IsActiveLine(int line_number) {
  return active_line.has_value() && active_line->line_number == line_number;
}

LLDBDebugger::BreakpointData& LLDBDebugger::GetBreakpointData(lldb::break_id_t id)
{
  auto it = id_breakpoint_data.find(id);
  if (it == id_breakpoint_data.end())
    throw std::runtime_error("Could not find breakpoint with id");
  return it->second;
}

LLDBDebugger::ExecResult LLDBDebugger::ExecCommand(const std::string& command, FileHierarchy& fh) {
  auto parsed_command = commandParser.Parse(command);
  switch (parsed_command.type) {
    case LLDB_CommandParser::ParsedCommandType::EMPTY:
      Logger::Info("Empty command");
      break;
    case LLDB_CommandParser::ParsedCommandType::BREAKPOINT_FILE_LINE:
      {
        auto bpfileline = std::get<LLDB_CommandParser::BPFileLine>(parsed_command.command);
        auto node = fh.GetElementByLocalPath(std::filesystem::path(bpfileline.file));
        if (!node) {
          Logger::Err("File '{}' does not exist in target", bpfileline.file);
        }
        else {
          if (AddBreakpoint(*node, bpfileline.line)) {
            Logger::Info("Breakpoint in file '{}' line {}", bpfileline.file, bpfileline.line);
          }
          else {
            Logger::Err("Failed to set breakpoint in file '{}' line {}", bpfileline.file, bpfileline.line);
          }
        }
        break;
      }
    case LLDB_CommandParser::ParsedCommandType::BREAKPOINT_SYMBOL:
      {
        Logger::ScopedGroup g("Breakpoint Symbol");
        auto bpsymbol = std::get<LLDB_CommandParser::BPSymbol>(parsed_command.command);
        auto functions = GetTarget().FindFunctions(bpsymbol.symbol.c_str());
        for (size_t i = 0; i < functions.GetSize(); i++) {
          // Check if the symbol context is ok
          lldb::SBSymbolContext symbolCtx = functions.GetContextAtIndex(i);
          if (!symbolCtx.IsValid()) { 
            Logger::Info("Symbol context not valid");
            continue;
          }
          Logger::Info("Symbol name: {}", symbolCtx.GetSymbol().GetName());

          // Check if the function is ok
          lldb::SBFunction func = symbolCtx.GetFunction();
          if (!func.IsValid()) {
            Logger::Err("Function not valid");
            continue;
          }
          Logger::Info("Function {}", func.GetName());

          lldb::SBAddress addr = func.GetStartAddress();
          lldb::SBLineEntry line = addr.GetLineEntry();
          GetTarget().ResolveSymbolContextForAddress(func.GetStartAddress(), lldb::eSymbolContextLineEntry);

          if (!line.IsValid()) {
            Logger::Err("Line entry not valid");
            continue;
          }

          auto fs = line.GetFileSpec();
          int lineno = line.GetLine();
          Logger::Info("Name: {}, Lineno: {}", fs.GetFilename(), lineno);

          auto node = fh.GetElementByLocalPath(std::filesystem::path(fs.GetFilename()));

          if (AddBreakpoint(*node, lineno)) {
            Logger::Info("Break on symbol {} in {}", bpsymbol.symbol, fs.GetFilename());
          }
          else {
            Logger::Crit("Failed to set breakpoint at symbol {} in {}", bpsymbol.symbol, fs.GetFilename());
          }
        }
        break;
      }
    case LLDB_CommandParser::ParsedCommandType::STEP:
      {
        GetProcess().GetSelectedThread().StepInto();
        Logger::Info("Step");
        break;
      }
    case LLDB_CommandParser::ParsedCommandType::NEXT:
      {
        GetProcess().GetSelectedThread().StepOver();
        Logger::Info("Next");
        break;
      }
    case LLDB_CommandParser::ParsedCommandType::CONTINUE:
      {
        GetProcess().Continue();
        Logger::Info("Continue");
        break;
      }
    case LLDB_CommandParser::ParsedCommandType::INVALID:
      Logger::Crit("Command {} is invalid", command);
      break;
    default:
      Logger::Warn("Command type: {} not implemented", (int)parsed_command.type);
  }

  return ExecResult::Ok();
}

void LLDBDebugger::DumpToStd(TempRedirect &redirect, std::ostream &out, size_t& offset)
{
    if (!redirect.file)
    {
        out << "[redirect file is null]" << std::endl;
        return;
    }

    fflush(redirect.file);
    fseek(redirect.file, offset, SEEK_SET);

    constexpr size_t buffer_size = 4096;
    char buffer[buffer_size];

    while (fgets(buffer, buffer_size, redirect.file) != nullptr)
    {
        out << buffer;
    }

    offset = ftell(redirect.file);

    out.flush();
}

void LLDBDebugger::LLDBEventThread() {
  using namespace lldb;
  SBEvent event;
  bool running = true;
  while (running) {
    DumpToStd(out_redirect, std::cout, out_offset);
    DumpToStd(err_redirect, std::cerr, err_offset);
    if (listener.WaitForEvent(1, event)) {
      if (SBProcess::EventIsProcessEvent(event)) {
        Logger::Info("Event name: {}", event.GetBroadcaster().GetName());
        StateType state = SBProcess::GetStateFromEvent(event);
        switch (state) {
          case eStateStopped: {
              Logger::Info("Target stopped");
              SBProcess process = SBProcess::GetProcessFromEvent(event);
              const uint32_t thread_count = process.GetNumThreads();

              for (uint32_t i = 0; i < thread_count; ++i) {
                  SBThread thread = process.GetThreadAtIndex(i);
                  if (!thread.IsValid()) {
                      continue;
                  }

                  Logger::Info("Thread ID: {} | Stop Reason: {}", thread.GetThreadID(), (uint64_t)thread.GetStopReason());

                  StopReason reason = thread.GetStopReason();
                  const size_t desc_count = thread.GetStopReasonDataCount();
                  std::string reason_str;

                  switch (reason) {
                      case eStopReasonBreakpoint: {
                          reason_str = "Breakpoint";
                          auto b_id = (lldb::break_id_t)thread.GetStopReasonDataAtIndex(0);
                          HitBreakpoint(b_id);
                          break;
                      }
                      case eStopReasonWatchpoint:
                          reason_str = "Watchpoint";
                          break;
                      case eStopReasonSignal:
                          reason_str = "Signal";
                          break;
                      case eStopReasonException:
                          reason_str = "Exception";
                          break;
                      case eStopReasonExec:
                          reason_str = "Exec";
                          break;
                      case eStopReasonThreadExiting:
                          reason_str = "Thread Exiting";
                          break;
                      case eStopReasonInstrumentation:
                          reason_str = "Instrumentation";
                          break;
                      default:
                          reason_str = "Other";
                          break;
                  }

                  Logger::Info("Reason: {} | Data Count: {}", reason_str, desc_count);

                  for (size_t j = 0; j < desc_count; ++j) {
                      Logger::Info("  Reason Data[{}] = {}", j, thread.GetStopReasonDataAtIndex(j));
                  }

                  const SBFrame frame = thread.GetFrameAtIndex(0);
                  if (frame.IsValid()) {
                      SBLineEntry line_entry = frame.GetLineEntry();
                      if (line_entry.IsValid()) {
                          const SBFileSpec file_spec = line_entry.GetFileSpec();
                          Logger::Info("  Location: {}:{}", file_spec.GetFilename(), line_entry.GetLine());
                      }
                  }
              }
              break;
          }
          case eStateExited:
            Logger::Info("Target exited");
            running = false;
            break;
          case eStateRunning:
            active_line.reset();
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
  DumpToStd(out_redirect, std::cout, out_offset);
  DumpToStd(err_redirect, std::cerr, err_offset);
  Logger::Info("LLDB Event Thread Stopping");
}
