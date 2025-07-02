#include "Init.hpp"
#include "Logger.hpp"
#include "Util.hpp"
#include <iostream>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "ImGuiIniParser.hpp"

namespace lldb_frontend {
  void err_callback(int error, const char* description) {
    std::cerr << "GLFWError: %s" << description << std::endl;
  }

  bool Init::isGlfwInit = false;
  bool Init::isGladInit = false;
  std::string Init::m_IniFilename = (Util::GetCurrentProgramDirectory() / "imgui.ini").string();
  void Init::InitGlad() {
    if (!isGlfwInit) {
      Logger::Err("Must call InitGlfw() before InitGlad()");
      exit(1);
    }

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
      Logger::Err("Error: Failed to load glad");
      exit(1);
    }
    isGladInit = true;
    Logger::Info("Initialized glad");
  }
  void Init::InitGlfw() {
    if (!glfwInit()) {
      Logger::Err("Error: Failed to glfwInit()");
      exit(1);
    }
    isGlfwInit = true;

    Logger::Info("Initialized glfw");
  }
  void Init::TerminateGlfw() {
    glfwTerminate();
  }

  void Init::InitImGui(const Window& window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = m_IniFilename.c_str();;
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)window.GetWindowHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Logger::Info("Initialized imgui");
  }
  void Init::DeinitImGui() {
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
    Logger::Info("Deinitialized imgui");
  }
  std::pair<size_t, size_t> Init::GetImGuiIniDimensions() {
    ImGuiIniParser imguiini;
    auto inipath = (Util::GetCurrentProgramDirectory() / "imgui.ini").string();
    bool rc = imguiini.LoadFile(inipath.c_str());
	  if (!rc)
      return {600, 600};
    auto size_str = std::string(imguiini.GetValue("Window][Dockspace Begin", "Size", "600,600"));
    auto com_pos = size_str.find(',');
    if (com_pos == std::string::npos)
      return {600, 600};
    auto width_str = size_str.substr(0, com_pos);
    auto height_str = size_str.substr(com_pos + 1, size_str.size() - com_pos);
    auto width = std::stoll(width_str);
    auto height = std::stoll(height_str);
    return {width, height};
  }
}
