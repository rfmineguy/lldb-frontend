#include "ImGuiLayer.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <tinyfiledialogs.h>
#include <iostream>
#include <fstream>
#include "Window.hpp"
#include "Util.hpp"
#include "Logger.hpp"

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
  DrawDebugWindow();
  DrawCodeWindow();
  DrawFileBrowser();
}

bool ImGuiLayer::LoadFile(const std::string& fullpath) {
  Logger::ScopedGroup g("ImGuiLayer::LoadFile");
  if (!fileContentsMap.contains(fullpath)) {
    Logger::Info("Loading: {}", fullpath);

    fileContentsMap[fullpath];
    std::ifstream f(fullpath);
    FileContext& ctx = fileContentsMap.at(fullpath);
    ctx.filename = fullpath;

    if (f.is_open()) {
      std::string line;
      while (std::getline(f, line)) {
        ctx.lines.emplace_back(line);
      }
      f.close();
    }
    else {
      Logger::Err("Failed to read {}", fullpath);
      Logger::EndGroup();
      return false;
    }
  }
  else {
    Logger::Info("{} already loaded", fullpath);
  }
  return true;
}

void ImGuiLayer::DrawDebugWindow() {
  ImGui::Begin("Debug");
  // Open File Dialog
  if (ImGui::Button("Open File Dialog")) {
    const char* fdpath = tinyfd_openFileDialog("Choose File", "", 0, NULL, "executables", 0);
    if (fdpath) {
      Logger::ScopedGroup g("OpenFileDialog");
      Logger::Info("Path: {}", fdpath);

      auto target = window_ref->GetDebuggerCtx()
        .GetDebugger()
        .CreateTarget(fdpath);

      window_ref->GetDebuggerCtx().GetDebugger().SetSelectedTarget(target);

      for (size_t i = 0; i < target.GetNumModules(); i++) {
        lldb::SBModule mod = target.GetModuleAtIndex(i);
        for (size_t j = 0; j < mod.GetNumCompileUnits(); j++) {
          lldb::SBCompileUnit cu = mod.GetCompileUnitAtIndex(j);
          auto directory = cu.GetFileSpec().GetDirectory();
          auto name = cu.GetFileSpec().GetFilename();

          fh.AddFile(directory, name);
        }
      }
      fh.Print();

      Util::PrintTargetModules(target);
      Util::PrintModuleCompileUnits(target, 0);
    }
  }

  // Load Example Exe
  if (ImGui::Button("Load Example Exe")) {
    auto target = window_ref->GetDebuggerCtx()
      .GetDebugger()
      .CreateTarget("/Users/rileyfischer/Documents/dev/lldb-frontend/build/lldb-frontend-test");

    window_ref->GetDebuggerCtx().GetDebugger().SetSelectedTarget(target);
    for (size_t i = 0; i < target.GetNumModules(); i++) {
      lldb::SBModule mod = target.GetModuleAtIndex(i);
      for (size_t j = 0; j < mod.GetNumCompileUnits(); j++) {
        lldb::SBCompileUnit cu = mod.GetCompileUnitAtIndex(j);
        auto directory = cu.GetFileSpec().GetDirectory();
        auto name = cu.GetFileSpec().GetFilename();

        fh.AddFile(directory, name);
      }
    }
    fh.Print();
    fh.GetRoot();

    Util::PrintTargetModules(target);
    Util::PrintModuleCompileUnits(target, 0);
  }
  ImGui::End();
}

void ImGuiLayer::DrawCodeWindow() {
  ImGui::Begin("Code Window");
  ImGui::End();
}

bool ImGuiLayer::ShowHeirarchyItem(const FileHeirarchy::HeirarchyElement* element) {
  bool isLeaf = element->children.empty();

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
  if (isLeaf) flags |= ImGuiTreeNodeFlags_Leaf;

  bool opened = ImGui::TreeNodeEx(element->local_path.c_str(), flags);
  if (isLeaf && ImGui::IsItemClicked(0)) {
    Logger::Info("Clicked {}", element->full_path.string());
    if (LoadFile(element->full_path)) {
    }
  }
  return opened;
}

void ImGuiLayer::FileHeirarchyRecursive(const FileHeirarchy::HeirarchyElement* element) {
  if (element == nullptr) return;
  if (ShowHeirarchyItem(element)) {
    for (auto& [key, child] : element->children) {
      FileHeirarchyRecursive(child);
    }
    ImGui::TreePop();
  }
}

void ImGuiLayer::DrawFileBrowser() {
  ImGui::Begin("File Browser");

  FileHeirarchyRecursive(fh.GetRoot());

  ImGui::End();
}
