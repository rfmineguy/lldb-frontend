#include "Window.hpp"
#include "Args.hpp"
#include "Logger.hpp"
#include "Util.hpp"
#include <filesystem>
#include <imgui.h>
#include <glad/gl.h>

Window::Window(const std::string& title, int width, int height):
  debuggerCtx(),
  imguiLayer(debuggerCtx)
{
  std::function<void(const LLDBDebugger::Event&)> listener = std::bind(&Window::DebuggerEventListener, this, std::placeholders::_1);
  debuggerCtx.SetEventCallback(listener);

  // Initialize glfw window
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  m_Window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
  if (!m_Window) {
    Logger::Err("Window: Failed to create window. Exitting... {}", glfwGetError(NULL));
    glfwTerminate();
    exit(2);
  }
  glfwMakeContextCurrent(m_Window);

  // Setup debugger context
  FileHierarchy& fh = imguiLayer.GetFileHierarchy();
  if (auto executable = lldb_frontend::Args::Get<std::string>("executable")) {
    debuggerCtx.SetTarget(debuggerCtx.GetDebugger().CreateTarget(executable->c_str()));
    auto target = debuggerCtx.GetTarget();
    std::filesystem::path fullpath;
    try {
      fullpath = std::filesystem::canonical(executable.value());
    }
    catch (const std::exception& e) {
      Logger::Err("Unable to find --executable " + fullpath.string());
      goto _exit;
    }

    for (size_t i = 0; i < target.GetNumModules(); i++) {
      lldb::SBModule mod = target.GetModuleAtIndex(i);
      for (size_t j = 0; j < mod.GetNumCompileUnits(); j++) {
        lldb::SBCompileUnit cu = mod.GetCompileUnitAtIndex(j);
        auto directory = cu.GetFileSpec().GetDirectory();
        auto name = cu.GetFileSpec().GetFilename();

        //NOTE: This is a temporary fix for a null pointer dereference
        // crash on windows
        if (!name || !directory) {
          Logger::Crit("Something went wrong. directory = '{}', name = '{}'", directory ? directory : "null", name ? name : "null");
          continue;
        } 

        fh.AddFile(std::filesystem::path(directory) / name);
      }
    }

    fh.ComputeTree();
    // fh.Print();

    Util::PrintTargetModules(target);
    Util::PrintModuleCompileUnits(target, 0);
  }

  // Auto exec
  if (auto autoexec = lldb_frontend::Args::Get<std::string>("autoexec")) {
    std::vector<Line> lines;
    if (Util::ReadFileLinesIntoVector(*autoexec, lines)) {
      for (const auto& line : lines) {
        Logger::Info("Line: {}", line.line);
        auto result = debuggerCtx.ExecCommand(line.line, fh);
      }
    }
  }
_exit:
  Logger::Info("Created window");
}

void Window::DebuggerEventListener(const LLDBDebugger::Event& event) {
  std::visit([](auto&& arg) {
    Logger::Info("Handling... {}", typeid(arg).name());
  }, event.data);
  if (auto e = std::get_if<LLDBDebugger::Event::LoadFile>(&event.data)) {
    imguiLayer.FrontendLoadFile(*e->node);
  }
  else if (auto e = std::get_if<LLDBDebugger::Event::IO>(&event.data)) {
    imguiLayer.PushIOLine(e->data);
  }
  else if (auto e = std::get_if<LLDBDebugger::Event::Continue>(&event.data)) {
    debuggerCtx.Continue();
  }
  else if (auto e = std::get_if<LLDBDebugger::Event::StepOver>(&event.data)) {
    debuggerCtx.StepOver();
  }
  else if (auto e = std::get_if<LLDBDebugger::Event::StepInto>(&event.data)) {
    debuggerCtx.StepInto();
  }
  else if (auto e = std::get_if<LLDBDebugger::Event::SwitchToFile>(&event.data)) {
    imguiLayer.SwitchToCodeFile(e->filepath);
  }
  else {
    std::visit([](auto&& arg) {
      Logger::Crit("Type not handled: {}", typeid(arg).name());
    }, event.data);
  }
}

Window::~Window() {
  glfwDestroyWindow(m_Window);
  Logger::Info("Destroyed window");
}

void Window::WindowLoop() {
  while (!glfwWindowShouldClose(m_Window)) {
    imguiLayer.Begin(this);
    imguiLayer.BeginDockspace();

    imguiLayer.Draw();
    ImGui::ShowDemoWindow();

    imguiLayer.EndDockspace();
    imguiLayer.DrawFilesNotFoundModal();
    imguiLayer.End();
    glfwPollEvents();
    glfwSwapBuffers(m_Window);
  }
}

const GLFWwindow* Window::GetWindowHandle() const {
  return m_Window;
}

LLDBDebugger& Window::GetDebuggerCtx() {
  return debuggerCtx;
}
