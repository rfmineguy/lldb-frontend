#ifndef IMGUI_STYLING_HPP
#define IMGUI_STYLING_HPP
#include <imgui.h>
#include <string>

namespace lldb_frontend {
  namespace Styling {
    enum LLDBFrontendCol {
      LLDBFrontendCol_EvenLine,                  // color of the even lines in the code view
      LLDBFrontendCol_OddLine,                   // color of the odd lines in the code view
      LLDBFrontendCol_BreakpointLineActive,      // color of active breakpoint lines
      LLDBFrontentCol_COUNT,
    };
    struct LLDBStyle {
      ImVec4 Colors[LLDBFrontentCol_COUNT];
    };

    static LLDBStyle lldbStyle;
    bool InitTheme();

    LLDBStyle& GetStyle();
  }
}

#endif
