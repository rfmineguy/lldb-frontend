#include "Window.hpp"
#include "Logger.hpp"
#include <imgui.h>
#include <glad/gl.h>

Window::Window(const std::string& title, int width, int height) {
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
