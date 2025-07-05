#include "Styling.hpp"
#include <imgui.h>

namespace lldb_frontend {
  Styling::LLDBStyle lldbStyle = Styling::LLDBStyle{};

  namespace Styling {
    void Init() {
      ImGuiStyle& style = ImGui::GetStyle();
      ImVec4* colors = style.Colors;

      style.WindowRounding = 6.0f;
      style.ChildRounding = 5.0f;
      style.FrameRounding = 4.0f;
      style.PopupRounding = 4.0f;
      style.GrabRounding = 4.0f;
      style.ScrollbarRounding = 4.0f;

      style.FramePadding = ImVec2(8, 5);
      style.ItemSpacing = ImVec2(10, 8);
      style.ItemInnerSpacing = ImVec2(6, 4);
      style.WindowPadding = ImVec2(10, 10);

      colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
      colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
      colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
      colors[ImGuiCol_ChildBg]               = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);
      colors[ImGuiCol_PopupBg]               = ImVec4(0.17f, 0.18f, 0.20f, 1.00f);
      colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
      colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

      colors[ImGuiCol_FrameBg]               = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
      colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.28f, 0.30f, 0.35f, 1.00f);
      colors[ImGuiCol_FrameBgActive]         = ImVec4(0.25f, 0.28f, 0.33f, 1.00f);

      colors[ImGuiCol_TitleBg]               = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
      colors[ImGuiCol_TitleBgActive]         = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
      colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);

      colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.17f, 1.00f);
      colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
      colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
      colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.45f, 1.00f);
      colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.55f, 1.00f);

      colors[ImGuiCol_CheckMark]             = ImVec4(0.33f, 0.85f, 0.55f, 1.00f);  // green
      colors[ImGuiCol_SliderGrab]            = ImVec4(0.33f, 0.85f, 0.55f, 1.00f);
      colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.28f, 0.78f, 0.5f, 1.00f);
      colors[ImGuiCol_Button]                = ImVec4(0.25f, 0.52f, 0.35f, 1.00f);
      colors[ImGuiCol_ButtonHovered]         = ImVec4(0.30f, 0.65f, 0.42f, 1.00f);
      colors[ImGuiCol_ButtonActive]          = ImVec4(0.33f, 0.75f, 0.50f, 1.00f);

      colors[ImGuiCol_Header]                = ImVec4(0.25f, 0.28f, 0.33f, 1.00f);
      colors[ImGuiCol_HeaderHovered]         = ImVec4(0.30f, 0.33f, 0.38f, 1.00f);
      colors[ImGuiCol_HeaderActive]          = ImVec4(0.35f, 0.38f, 0.43f, 1.00f);

      colors[ImGuiCol_Separator]             = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);
      colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.33f, 0.36f, 0.40f, 1.00f);
      colors[ImGuiCol_SeparatorActive]       = ImVec4(0.36f, 0.39f, 0.43f, 1.00f);

      colors[ImGuiCol_ResizeGrip]            = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
      colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.36f, 0.36f, 0.38f, 1.00f);
      colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.40f, 0.40f, 0.43f, 1.00f);

      colors[ImGuiCol_Tab]                   = ImVec4(0.17f, 0.19f, 0.22f, 1.00f);
      colors[ImGuiCol_TabHovered]            = ImVec4(0.25f, 0.28f, 0.33f, 1.00f);
      colors[ImGuiCol_TabActive]             = ImVec4(0.22f, 0.25f, 0.30f, 1.00f);
      colors[ImGuiCol_TabUnfocused]          = ImVec4(0.15f, 0.17f, 0.20f, 1.00f);
      colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.18f, 0.20f, 0.23f, 1.00f);
      style.FrameRounding = 4;

      LLDBStyle& lldbStyle = GetStyle();
      lldbStyle.Colors[LLDBFrontendCol_EvenLine] = colors[ImGuiCol_Tab];
      lldbStyle.Colors[LLDBFrontendCol_OddLine]  = colors[ImGuiCol_TabUnfocused];
      lldbStyle.Colors[LLDBFrontendCol_BreakpointLineActive] = ImVec4(0.35f, 0.10f, 0.10f, 1.0f);
    }
    LLDBStyle& GetStyle() {
      return lldbStyle;
    }
  }
}
