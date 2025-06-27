#include "Window.hpp"
#include <imgui.h>
#include <iostream>
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
    std::cerr << "Window: Failed to create window. Exitting..." << glfwGetError(NULL) << std::endl;
    glfwTerminate();
    exit(2);
  }
  glfwMakeContextCurrent(m_Window);
  std::cout << "Created window" << std::endl;
}

Window::~Window() {
  glfwDestroyWindow(m_Window);
  std::cout << "Destroyed window" << std::endl;
}

void Window::WindowLoop() {
  while (!glfwWindowShouldClose(m_Window)) {
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(m_Window);
    glfwPollEvents();
  }
}

const GLFWwindow* Window::GetWindowHandle() const {
  return m_Window;
}
