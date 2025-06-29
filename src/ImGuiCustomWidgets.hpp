#ifndef CUSTOM_IMGUI_WIDGETS_HPP
#define CUSTOM_IMGUI_WIDGETS_HPP

struct Line;
struct FileContext;
struct ImGuiLayer;

namespace ImGuiCustom {
  void Breakpoint(int id, Line& line, FileContext& fctx, ImGuiLayer& imguiLayer);
}

#endif
