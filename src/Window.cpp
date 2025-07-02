#include "Window.hpp"
#include "Args.hpp"
#include "Logger.hpp"
#include "Util.hpp"
#include <filesystem>
#include <imgui.h>
#include <glad/gl.h>

Window::Window(const std::string& title, int width, int height):
  imguiLayer(debuggerCtx)
{
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
  if (auto executable = lldb_frontend::Args::Get<std::string>("executable")) {
    debuggerCtx.SetTarget(debuggerCtx.GetDebugger().CreateTarget(executable->c_str()));
    auto target = debuggerCtx.GetTarget();
    auto fullpath = std::filesystem::canonical(executable->c_str());

    auto working_dir = Util::GetTargetSourceRootDirectory(fullpath)->string();
    FileHeirarchy& fh = imguiLayer.GetFileHeirarchy();
    fh.SetWorkingDir(working_dir);

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

        fh.AddFile(directory, name);
      }
    }
    fh.Print();

    Util::PrintTargetModules(target);
    Util::PrintModuleCompileUnits(target, 0);
  }
  Logger::Info("Created window");
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
