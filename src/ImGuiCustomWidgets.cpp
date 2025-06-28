#include "ImGuiCustomWidgets.hpp"
#include <imgui.h>

namespace ImGuiCustom {
  void Breakpoint(int id, bool& state) {
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    float radius = 6.0f;
    float diameter = radius * 2.0f;

    // Create an invisible button for interaction
    ImGui::InvisibleButton("circle_checkbox", ImVec2(diameter + 4, diameter + 4));
    if (ImGui::IsItemClicked())
      state = !state;

    // Get draw list and draw circle
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 center = ImVec2(cursorPos.x + radius + 2, cursorPos.y + radius + 2);

    // Draw outer circle
    drawList->AddCircle(center, radius, IM_COL32(255, 255, 255, 255), 16, 1.5f);

    // If checked, draw filled circle
    if (state) {
      drawList->AddCircleFilled(center, radius - 2.0f, IM_COL32(255, 255, 255, 255), 16);
    }
  }
}
