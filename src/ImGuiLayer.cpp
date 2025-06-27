#include "ImGuiLayer.hpp"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

ImGuiLayer::ImGuiLayer() {}
ImGuiLayer::~ImGuiLayer() {}

void ImGuiLayer::Begin(Window* window) {
  window_ref = window;
  ImGui_ImplGlfw_NewFrame();
  ImGui_ImplOpenGL3_NewFrame();

  // Enable docking for the next frame
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  
  ImGui::NewFrame();
}
void ImGuiLayer::End() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void ImGuiLayer::BeginDockspace() {
  static bool opt_fullscreen = true;
  static bool opt_is_open = true;
  static bool opt_padding = false;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen) {
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus; 
  }
  else {
    dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;
  
  if (!opt_padding) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
  }
  ImGui::Begin("Dockspace Begin", &opt_is_open, window_flags);
  if (!opt_padding) {
    ImGui::PopStyleVar();
  }
  if (opt_fullscreen) {
    ImGui::PopStyleVar(2);
  }
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("DockspaceID");
    ImGui::DockSpace(dockspace_id, ImVec2(0.f, 0.f), dockspace_flags);
  }
  else {
    // Docking is disabled
    // NOTE: Not sure how to recover or what to do here
  }
}
void ImGuiLayer::EndDockspace() {
  ImGui::End();
}
void ImGuiLayer::Draw() {
}
