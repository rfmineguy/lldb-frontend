#ifndef CUSTOM_IMGUI_WIDGETS_HPP
#define CUSTOM_IMGUI_WIDGETS_HPP
#include "FileHierarchy.hpp"

struct Line;
struct FileContext;
struct ImGuiLayer;

namespace ImGuiCustom {
  void Breakpoint(int id, FileHierarchy::TreeNode& node, ImGuiLayer& imguiLayer, bool active);
}

#endif
