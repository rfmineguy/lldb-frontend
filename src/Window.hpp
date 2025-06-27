#ifndef WINDOW_HPP
#define WINDOW_HPP
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include "ImGuiLayer.hpp"
#include "LLDBDebugger.hpp"

class Window {
  public:
    Window(const std::string& title, int width, int height);
    ~Window();

    void WindowLoop();
    const GLFWwindow* GetWindowHandle() const;
    const ImGuiLayer* GetImGuiLayer();
    LLDBDebugger&     GetDebuggerCtx();

  private:
    

  private:
    GLFWwindow *m_Window;
    ImGuiLayer imguiLayer;
    LLDBDebugger debuggerCtx;
};

#endif
