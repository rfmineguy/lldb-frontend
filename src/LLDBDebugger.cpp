#include "LLDBDebugger.hpp"

LLDBDebugger::LLDBDebugger() {
    lldb::SBDebugger::Initialize();
    debugger = lldb::SBDebugger::Create();
    debugger.SetAsync(false);
}

LLDBDebugger::~LLDBDebugger() {
  lldb::SBDebugger::Destroy(debugger);
  lldb::SBDebugger::Terminate();
}

lldb::SBDebugger& LLDBDebugger::GetDebugger() {
  return debugger;
}

lldb::SBTarget& LLDBDebugger::GetTarget() {
  return target;
}

LLDBDebugger::ExecResult LLDBDebugger::ExecCommand(const std::string& command) {
  return ExecResult::Ok;
}
