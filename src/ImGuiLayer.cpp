#include "ImGuiLayer.hpp"
#include "ImGuiCustomWidgets.hpp"
#include "LLDBDebugger.hpp"
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
#include "Resources.hpp"

ImGuiLayer::ImGuiLayer(LLDBDebugger& debugger):
  debugger(debugger)
{}
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

LLDBDebugger& ImGuiLayer::GetDebugger()
{
  return debugger;
}

FileHierarchy& ImGuiLayer::GetFileHierarchy() {
  return fh;
}

bool ImGuiLayer::LoadFile(FileHierarchy::TreeNode& node) {
  Logger::ScopedGroup g("ImGuiLayer::LoadFile");
  if (node.lines.has_value()) {
    Logger::Info("{} already loaded", node.path.string());
    return true;
  }
  node.lines = std::vector<Line>{0, Line{}};
  Logger::Info("Loading: {}", node.path.string());
  Util::ReadFileLinesIntoVector(node.path, *node.lines);
  Logger::Info("Loaded");
  return true;
}

void ImGuiLayer::DrawDebugWindow() {
  ImGui::Begin("Debug");
  // Open File Dialog
  if (ImGui::Button("Open File Dialog")) {
    const char* fdpath = tinyfd_openFileDialog("Choose File", "", 0, NULL, "executables", 0);
    std::filesystem::path path(fdpath);
    if (fdpath) {
      Logger::ScopedGroup g("OpenFileDialog");
      Logger::Info("Path: {}", fdpath);

      auto& dctx = window_ref->GetDebuggerCtx();

      dctx.SetTarget(window_ref->GetDebuggerCtx()
        .GetDebugger()
        .CreateTarget(fdpath));

      auto target = dctx.GetTarget();
      auto source_directory = Util::GetTargetSourceRootDirectory(path);
      if (!source_directory.has_value()) {
        source_directory = path.root_path();
      }

      for (size_t i = 0; i < target.GetNumModules(); i++) {
        lldb::SBModule mod = target.GetModuleAtIndex(i);
        for (size_t j = 0; j < mod.GetNumCompileUnits(); j++) {
          lldb::SBCompileUnit cu = mod.GetCompileUnitAtIndex(j);
          auto directory_c_str = cu.GetFileSpec().GetDirectory();
          auto directory = std::filesystem::path(directory_c_str);
          auto name = cu.GetFileSpec().GetFilename();

          fh.AddFile(directory / name);
        }
      }
      fh.ComputeTree();

      Util::PrintTargetModules(target);
      Util::PrintModuleCompileUnits(target, 0);
    }
  }

  // Load Example Exe
  if (ImGui::Button("Load Example Exe")) {
    auto program_directory = Util::GetCurrentProgramDirectory();

    auto target_path = (program_directory / "lldb-frontend-test").string();

#ifdef _WIN32
    target_path += ".exe";
#endif

    auto& dctx = window_ref->GetDebuggerCtx();

    Logger::Info("TargetPath: {}", target_path);
    dctx.SetTarget(dctx.GetDebugger().CreateTarget(target_path.c_str()));

    auto target = dctx.GetTarget();
    auto t_path = std::filesystem::path(target_path);
    auto source_directory = Util::GetTargetSourceRootDirectory(t_path);
    if (!source_directory.has_value()) {
      source_directory = t_path.root_path();
    }

    for (size_t i = 0; i < target.GetNumModules(); i++) {
      lldb::SBModule mod = target.GetModuleAtIndex(i);
      for (size_t j = 0; j < mod.GetNumCompileUnits(); j++) {
        lldb::SBCompileUnit cu = mod.GetCompileUnitAtIndex(j);
        auto directory_c_str = cu.GetFileSpec().GetDirectory();
        auto directory = std::filesystem::path(directory_c_str);
        auto name = cu.GetFileSpec().GetFilename();
        if (name)
          fh.AddFile(directory / name);
      }
    }
    fh.ComputeTree();

    Util::PrintTargetModules(target);
    Util::PrintModuleCompileUnits(target, 0);

    // dctx.LaunchTarget();
  }
  ImGui::End();
}

void ImGuiLayer::DrawCodeFile(FileHierarchy::TreeNode& node) {
  auto node_path_string = node.path.string();
  bool active_file = debugger.IsActiveFile(node_path_string.c_str());
  if (node.lines->empty()) return;
  for (int i = 0; i < node.lines->size(); i++) {
    auto& line = node.lines->at(i);
    ImGui::PushID(i);
    auto line_number = i + 1;
    auto line_active = debugger.IsActiveLine(line_number);
    ImGuiCustom::Breakpoint(i, node, *this, line_active); ImGui::SameLine();
    ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImVec2 text_size = ImGui::CalcTextSize(line.line.c_str());
    ImVec2 line_size = ImVec2(ImGui::GetWindowWidth() - ImGui::GetStyle().WindowPadding.x * 1.75, text_size.y * 1.5);
    auto line_bg_color = i % 2 == 0 ? ImGui::GetColorU32(ImGuiCol_Button) : ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    if (active_file && line_active)
      line_bg_color = IM_COL32(248, 42, 128, 255);
    ImGui::GetWindowDrawList()->AddRectFilled(
        cursor,
        cursor + line_size,
        line_bg_color,
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
      auto local_path_string = file->name;
      if (ImGui::BeginTabItem(local_path_string.c_str())) {
        DrawCodeFile(*file);
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
  if (ImGui::Button("Run")) {
    window_ref->GetDebuggerCtx().LaunchTarget();
  }
  if (ImGui::Button("Continue")) {
    window_ref->GetDebuggerCtx().GetProcess().Continue();
  }
  if (ImGui::Button("Step Into")) {
    window_ref->GetDebuggerCtx().GetProcess().GetSelectedThread().StepInto();
  }
  if (ImGui::Button("Step Over")) {
    window_ref->GetDebuggerCtx().GetProcess().GetSelectedThread().StepOver();
  }
  if (ImGui::Button("Next")) {
    window_ref->GetDebuggerCtx().GetProcess().GetSelectedThread().StepOver(); // equivalent to next i guess
  }
  ImGui::End();
}

bool ImGuiLayer::ShowHierarchyItem(FileHierarchy::TreeNode& node, const std::filesystem::path& parent_path, const std::filesystem::path& path) {
  std::string tex_id;
  auto type = FileHierarchy::GetTypeFromNode(node);
  switch (type) {
    case FileHierarchy::TreeNodeType::FOLDER: tex_id = "folder"; break;
    case FileHierarchy::TreeNodeType::FILE:   tex_id = "file"; break;
    default: tex_id = "unknown"; break;
  }
  bool isLeaf = type == FileHierarchy::TreeNodeType::FILE;

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
  if (isLeaf) flags |= ImGuiTreeNodeFlags_Leaf;

  Texture* tex = nullptr;
  if (auto tex = lldb_frontend::Resources::GetTexture(tex_id)) {
    auto v = tex->GetTextureId();
    ImGui::ImageWithBg((ImTextureID)(intptr_t)v, tex->GetImGuiSizeScaled(0.1f));
    ImGui::SameLine();
  }
  std::string remainder = path.string().substr(parent_path.string().length());
  bool opened = ImGui::TreeNodeEx(remainder.c_str(), flags);
  if (isLeaf && ImGui::IsItemClicked(0)) {
    Logger::Info("Clicked {}", path.string());
    node.LoadFromDisk();
    FrontendLoadFile(node);
  }
  return opened;
}

void ImGuiLayer::FileHierarchyRecursive(const std::filesystem::path& parent_path, FileHierarchy::TreeNode& node) {
  auto [lookahead_path, lookahead_node_ptr] = node.LookaheadPath();
  auto& lookahead_node = *lookahead_node_ptr;
  if (ShowHierarchyItem(lookahead_node, parent_path, lookahead_path)) {
    for (auto& [key, child] : lookahead_node.children)
      FileHierarchyRecursive(lookahead_path, child);
    ImGui::TreePop();
  }
}

void ImGuiLayer::DrawFileBrowser() {
  ImGui::Begin("File Browser");

  auto& root = fh.GetRoot();

  if (root.name.empty() && root.children.empty()) {
    ImGui::End();
    return;
  }

  for (auto& [key, child] : root.children)
    FileHierarchyRecursive(std::filesystem::path(""), child);

  ImGui::End();
}

void ImGuiLayer::DrawBreakpointsWindow() {
  ImGui::Begin("Breakpoints");
  auto& dctx = window_ref->GetDebuggerCtx();
  auto target = dctx.GetTarget();
  int numBreakpoints = target.GetNumBreakpoints();
  for (int i = 0; i < numBreakpoints; i++) {
    auto bp = target.GetBreakpointAtIndex(i);
    auto id = bp.GetID();
    auto& b_data = dctx.GetBreakpointData(id);
    ImGui::Text("%d: %s:%d", id, b_data.filename.c_str(), b_data.line_number);
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
      auto result = window_ref->GetDebuggerCtx().ExecCommand(inputBuf, fh);
      switch (result.status) {
        case LLDBDebugger::ExecResultStatus::Ok:
          Logger::Todo("ExecResult::Ok");
          break;
        default:
          Logger::Crit("ExecResult::Fail => {}", result.message);
      }
      reclaim_focus = true;
  }


  ImGui::End();
}

void ImGuiLayer::DrawFilesNotFoundModal()
{
  if (m_FilesNotFoundModal_open)
  {
    ImGui::OpenPopup("FilesNotFound");

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("FilesNotFound", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("The following files were not found:");
        ImGui::Separator();

        for (auto& element : m_FilesNotFoundModal_files)
        {
          static int clicked = 0;
          if (ImGui::Button(element->name.c_str()))
              clicked++;
          if (clicked & 1)
          {
              ImGui::SameLine();
              ImGui::Text("Thanks for clicking me!");
          }
        }

        if (ImGui::Button("OK", ImVec2(120, 0))) {
          m_FilesNotFoundModal_open = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
          m_FilesNotFoundModal_open = false;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
  }
}

bool ImGuiLayer::FrontendLoadFile(FileHierarchy::TreeNode& node) {
  if (LoadFile(node)) {
    if (std::find(openFiles.begin(), openFiles.end(), &node) == openFiles.end()) {
      openFiles.push_back(&node);
    }
    return true;
  }
  else {
    m_FilesNotFoundModal_open = true;
    m_FilesNotFoundModal_files.push_back(&node);
    return false;
  }
}