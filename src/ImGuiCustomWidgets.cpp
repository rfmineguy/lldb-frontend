
namespace ImGuiCustom {
  void Breakpoint(int id, FileHeirarchy::HeirarchyElement& element, ImGuiLayer& imguiLayer) {
    if (element.lines->empty()) return;
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    float radius = 6.0f;
    float diameter = radius * 2.0f;

    // Create an invisible button for interaction
    ImGui::InvisibleButton("circle_checkbox", ImVec2(diameter + 4, diameter + 4));
    if (ImGui::IsItemClicked())
    {
      auto& debugger = imguiLayer.GetDebugger();
      auto actioned = (!element.lines->at(id).bp) ?
        debugger.AddBreakpoint(element, id) :
        debugger.RemoveBreakpoint(element, id);
    }

    // Get draw list and draw circle
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 center = ImVec2(cursorPos.x + radius + 2, cursorPos.y + radius + 2);

    // Draw outer circle
    drawList->AddCircle(center, radius, IM_COL32(255, 255, 255, 255), 16, 1.5f);

    // If checked, draw filled circle
    if (element.lines->at(id).bp) {
      drawList->AddCircleFilled(center, radius - 2.0f, IM_COL32(255, 255, 255, 255), 16);
    }
  }
}
