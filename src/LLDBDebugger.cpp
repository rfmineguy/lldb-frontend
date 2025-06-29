#include "LLDBDebugger.hpp"
#include "FileContext.hpp"
#include <filesystem>
#include <stdexcept>

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

    const char* filename = element.full_path.c_str();
    int line_number = id + 1;
    lldb::SBBreakpoint bp = target.BreakpointCreateByLocation(filename, line_number);

    if (bp.IsValid()) {
        line.bp = true;
        line.bp_id = bp.GetID();
        auto real_filename = std::filesystem::path(element.full_path).filename().string();
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
