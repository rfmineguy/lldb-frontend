#ifndef CUSTOM_IMGUI_WIDGETS_HPP
#define CUSTOM_IMGUI_WIDGETS_HPP
#include "FileHeirarchy.hpp"

struct Line;
struct FileContext;
struct ImGuiLayer;

namespace ImGuiCustom {
  void Breakpoint(int id, FileHeirarchy::HeirarchyElement& element, ImGuiLayer& imguiLayer, bool active);
}

#endif
