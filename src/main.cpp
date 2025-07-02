#include <argparse/argparse.hpp>
#include <iostream>
#include "Init.hpp"
#include "Resources.hpp"
#include "Window.hpp"
#include "Args.hpp"

int main(int argc, char** argv) {
  //  lldb::SBDebugger::Initialize();
  //  lldb::SBDebugger debugger = lldb::SBDebugger::Create();
  //  
  //  debugger.SetAsync(false);
  //  std::string targetExeFile = "lldb-frontend-test";
  //  std::string targetExeMain = "test.cpp";

  //  lldb::SBTarget target = debugger.CreateTarget(targetExeFile.c_str());
  //  if (!target.IsValid()) {
  //    std::cerr << "Failed to create target: " << targetExeFile << std::endl;
  //    return 1;
  //  }

  //  // Set a breakpoint at main.cpp line 10 (adjust to your source)
  //  lldb::SBBreakpoint breakpoint = target.BreakpointCreateByLocation(targetExeMain.c_str(), 4);
  //  std::cout << "Breakpoint set: " << breakpoint.GetID() << std::endl;

  //  // Launch the process
  //  lldb::SBError error;
  //  lldb::SBProcess process = target.LaunchSimple(nullptr, nullptr, ".");
  //  if (!process.IsValid() || error.Fail()) {
  //      std::cerr << "Failed to launch process: " << error.GetCString() << "\n";
  //      return 1;
  //  }

  //  std::cout << "Process launched with PID: " << process.GetProcessID() << std::endl;

  //  // Cleanup
  //  lldb::SBDebugger::Terminate();

  //  std::cout << "Hello World" << std::endl;

  lldb_frontend::Args::SetupOptions();
  if (!lldb_frontend::Args::Parse(argc, argv)) {
    std::cerr << "Failed to parse arguments" << std::endl;
    return -1;
  }

  if (auto value = lldb_frontend::Args::Get<std::string>("executable")) {
    std::cout << "Provided executable: " << *value << std::endl;
  }

  lldb_frontend::Init::InitGlfw();

  auto [width, height] = lldb_frontend::Init::GetImGuiIniDimensions();

  Window w("LLDB Frontend", width, height);
  lldb_frontend::Init::InitGlad();
  lldb_frontend::Resources::LoadAll();
  lldb_frontend::Init::InitImGui(w);
  w.WindowLoop();
  lldb_frontend::Init::DeinitImGui();
  lldb_frontend::Init::TerminateGlfw();
}
