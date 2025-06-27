#ifndef WINDOW_HPP
#define WINDOW_HPP
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>

class Window {
  public:
    Window(const std::string& title, int width, int height);
    ~Window();

    void WindowLoop();
    const GLFWwindow* GetWindowHandle() const;

  private:
    

  private:
    GLFWwindow *m_Window;
};

#endif
