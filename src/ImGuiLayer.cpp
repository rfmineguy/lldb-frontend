#include "ImGuiLayer.hpp"
#include "ImGuiCustomWidgets.hpp"
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
  DrawStackTraceWindow();
  DrawControlsWindow();
  DrawBreakpointsWindow();
  DrawLLDBCommandWindow();
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

void ImGuiLayer::DrawCodeFile(FileContext& fctx) {
  for (int i = 0; i < fctx.lines.size(); i++) {
    auto& line = fctx.lines.at(i);
    ImGui::PushID(i);
    ImGuiCustom::Breakpoint(i, line.bp); ImGui::SameLine();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 text_size = ImGui::CalcTextSize(line.line.c_str());
    ImVec2 line_size = ImVec2(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 1.75, text_size.y * 1.5);
    ImGui::GetWindowDrawList()->AddRectFilled(
        cursor,
        cursor + line_size,
        i % 2 == 0 ? ImGui::GetColorU32(ImGuiCol_Button) : ImGui::GetColorU32(ImGuiCol_ButtonHovered),
        0.f);
    ImGui::Text("[%d] | %s", i, line.line.c_str());
    ImGui::PopID();
  }
}

void ImGuiLayer::DrawCodeWindow() {
  ImGui::Begin("Code Window");
  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
  if (ImGui::BeginTabBar("Code File Tabs", tab_bar_flags)) {
    for (auto& file : openFiles) {
      if (ImGui::BeginTabItem(file->local_path.c_str())) {
        DrawCodeFile(fileContentsMap.at(file->full_path));
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}

void ImGuiLayer::DrawStackTraceWindow() {
  ImGui::Begin("Stack Trace");
  ImGui::End();
}

void ImGuiLayer::DrawControlsWindow() {
  ImGui::Begin("Controls");
  if (ImGui::Button("Continue")) {}
  if (ImGui::Button("Step Into")) {}
  if (ImGui::Button("Step Over")) {}
  if (ImGui::Button("Next")) {}
  ImGui::End();
}

bool ImGuiLayer::ShowHeirarchyItem(const FileHeirarchy::HeirarchyElement* element) {
  bool isLeaf = element->children.empty();

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
  if (isLeaf) flags |= ImGuiTreeNodeFlags_Leaf;

  bool opened = ImGui::TreeNodeEx(element->local_path.string().c_str(), flags);
  if (isLeaf && ImGui::IsItemClicked(0)) {
    Logger::Info("Clicked {}", element->full_path.string());
    if (LoadFile(element->full_path)) {
      if (std::find(openFiles.begin(), openFiles.end(), element) == openFiles.end()) {
        openFiles.push_back((FileHeirarchy::HeirarchyElement*)element);
      }
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

void ImGuiLayer::DrawBreakpointsWindow() {
  ImGui::Begin("Breakpoints");
  const auto& target = window_ref->GetDebuggerCtx().GetTarget();
  int numBreakpoints = target.GetNumBreakpoints();
  for (int i = 0; i < numBreakpoints; i++) {
    auto bp = target.GetBreakpointAtIndex(i);
    auto id = bp.GetID();
    ImGui::Text("%d", id);
  }
  ImGui::End();
}

int ImGuiLayer::TextEditCallbackStub(ImGuiInputTextCallbackData* data) {
  ImGuiLayer* _this = (ImGuiLayer*)data->UserData;
  return 1;
}

void ImGuiLayer::DrawLLDBCommandWindow() {
  static char inputBuf[256];
  static ImVector<std::string> items;
  ImGui::Begin("Command Window");

  const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
  if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar)) {
    for (const auto& item : items) {
      ImGui::TextUnformatted(item.c_str());
    }
  }
  ImGui::EndChild();

  // Input field
  bool reclaim_focus = false;
  ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
  if (ImGui::InputText("Input", inputBuf, IM_ARRAYSIZE(inputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
  {
      items.push_back(std::string(inputBuf));
      auto result = window_ref->GetDebuggerCtx().ExecCommand(inputBuf);
      switch (result) {
        case LLDBDebugger::ExecResult::Ok:
          Logger::Todo("ExecResult::Ok unimplemented");
          break;
        default:
          Logger::Crit("ExecCommand failed {}", (int)result);
      }
      reclaim_focus = true;
  }


  ImGui::End();
}
