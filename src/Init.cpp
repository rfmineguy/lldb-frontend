#include "Init.hpp"
#include <iostream>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

namespace lldb_frontend {
  void err_callback(int error, const char* description) {
    std::cerr << "GLFWError: %s" << description << std::endl;
  }

  bool Init::isGlfwInit = false;
  bool Init::isGladInit = false;
  void Init::InitGlad() {
    if (!isGlfwInit) {
      std::cerr << "Error: Must call InitGlfw() before InitGlad()" << std::endl;
      exit(1);
    }

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
      std::cout << "Error: Failed to load glad" << std::endl;
      exit(1);
    }
    isGladInit = true;
    std::cout << "Initialized glad" << std::endl;
  }
  void Init::InitGlfw() {
    if (!glfwInit()) {
      std::cerr << "Error: Failed to glfwInit()" << std::endl;
      exit(1);
    }
    isGlfwInit = true;

    std::cout << "Initialized glfw" << std::endl;
  }
  void Init::TerminateGlfw() {
    glfwTerminate();
  }

  void Init::InitImGui(const Window& window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window.GetWindowHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::cout << "Initialized imgui" << std::endl;
  }
  void Init::DeinitImGui() {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    std::cout << "Deinitialized imgui" << std::endl;
  }
}
