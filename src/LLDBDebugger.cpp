#include "LLDBDebugger.hpp"
#include "FileContext.hpp"
#include <filesystem>

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

lldb::SBTarget LLDBDebugger::GetTarget() {
  return debugger.GetSelectedTarget();
}

void LLDBDebugger::SetTarget(lldb::SBTarget target)
{
  debugger.SetSelectedTarget(target);
}

bool LLDBDebugger::AddBreakpoint(FileContext& fctx, int id)
{
    if (id < 0 || id >= static_cast<int>(fctx.lines.size()))
    {
        return false;
    }

    Line& line = fctx.lines[id];
    if (line.bp)
    {
        return false;
    }

    auto target = GetTarget();

    const char* filename = fctx.filename.c_str();
    int line_number = id + 1;
    lldb::SBBreakpoint bp = target.BreakpointCreateByLocation(filename, line_number);

    if (bp.IsValid())
    {
        line.bp = true;
        line.bp_id = bp.GetID(); // 🔥 track this
        return true;
    }

    return false;
}

bool LLDBDebugger::RemoveBreakpoint(FileContext& fctx, int id)
{
    if (id < 0 || id >= static_cast<int>(fctx.lines.size()))
    {
        return false;
    }

    Line& line = fctx.lines[id];
    if (!line.bp || line.bp_id == LLDB_INVALID_BREAK_ID)
    {
        return false;
    }

    auto target = GetTarget();
    if (target.BreakpointDelete(line.bp_id))
    {
        line.bp = false;
        line.bp_id = LLDB_INVALID_BREAK_ID;
        return true;
    }

    return false;
}

LLDBDebugger::ExecResult LLDBDebugger::ExecCommand(const std::string& command) {
  return ExecResult::Ok;
}
