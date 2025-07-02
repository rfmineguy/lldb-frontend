#include "ImGuiCustomWidgets.hpp"
#include "ImGuiLayer.hpp"
#include "FileHierarchy.hpp"
#include "LLDBDebugger.hpp"
#include <imgui.h>

namespace ImGuiCustom {
  void Breakpoint(int id, FileHierarchy::TreeNode& node, ImGuiLayer& imguiLayer, bool active) {
    if (node.lines->empty()) return;
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    float radius = 6.0f;
    float diameter = radius * 2.0f;

    // Create an invisible button for interaction
    ImGui::InvisibleButton("circle_checkbox", ImVec2(diameter + 4, diameter + 4));
    if (ImGui::IsItemClicked())
    {
      auto& debugger = imguiLayer.GetDebugger();
      auto actioned = (!node.lines->at(id).bp) ?
        debugger.AddBreakpoint(node, id) :
        debugger.RemoveBreakpoint(node, id);
    }

    // Get draw list and draw circle
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 center = ImVec2(cursorPos.x + radius + 2, cursorPos.y + radius + 2);

    // Draw outer circle
    auto circle_color = active ? 
      IM_COL32(248, 42, 128, 255) :
      IM_COL32(255, 255, 255, 255);
    drawList->AddCircle(center, radius, circle_color, 16, 1.5f);

    // If checked, draw filled circle
    if (node.lines->at(id).bp) {
      drawList->AddCircleFilled(center, radius - 2.0f, circle_color, 16);
    }
  }
}
