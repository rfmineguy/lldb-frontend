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
  if (!GetTarget().IsValid()) {
    Logger::Crit("Failed to launch target. Target not valid. {}", target.GetExecutable().GetFilename());
    return;
  }

  // Setup launch info
  lldb::SBLaunchInfo launch_info(nullptr);
  int out_fn, in_fn, err_fn;

#ifdef _WIN32
  out_fn = _fileno(stdout);
  in_fn = _fileno(stdin);
  err_fn = _fileno(stderr);
#else
  out_fn = STDOUT_FILENO;
  in_fn = STDIN_FILENO;
  err_fn = STDERR_FILENO;
#endif
  launch_info.AddDuplicateFileAction(out_fn, out_fn);
  launch_info.AddDuplicateFileAction(err_fn, err_fn);
  launch_info.AddDuplicateFileAction(in_fn, in_fn);

  lldb::SBError error;
  process = GetTarget().Launch(launch_info, error);
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

lldb::SBProcess LLDBDebugger::GetProcess() {
  return process;
}

void LLDBDebugger::SetTarget(lldb::SBTarget target) {
  debugger.SetSelectedTarget(target);
}

bool LLDBDebugger::AddBreakpoint(FileHeirarchy::HeirarchyElement& element, int id) {
  if (element.lines->empty()) {
    // If we try to add a breakpoint and the file hasnt been loaded yet, we have to load it
    //   so that we have access to its lines
    // There might be a better way to do this, but for now this works fine.
    element.LoadFromDisk();
  }

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
        Logger::Info("Set breakpoint at {} on line {}", filename, line_number);
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

LLDBDebugger::ExecResult LLDBDebugger::ExecCommand(const std::string& command, FileHeirarchy& fh) {
  auto parsed_command = commandParser.Parse(command);
  switch (parsed_command.type) {
    case LLDB_CommandParser::ParsedCommandType::EMPTY:
      Logger::Info("Empty command");
      break;
    case LLDB_CommandParser::ParsedCommandType::BREAKPOINT_FILE_LINE:
      {
        auto bpfileline = std::get<LLDB_CommandParser::BPFileLine>(parsed_command.command);
        auto element = fh.GetElementByLocalPath(bpfileline.file);
        if (!element) {
          Logger::Err("File '{}' does not exist in target", bpfileline.file);
        }
        else {
          if (AddBreakpoint(*element, bpfileline.line)) {
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

          auto element = fh.GetElementByLocalPath(fs.GetFilename());

          if (AddBreakpoint(*element, lineno)) {
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
