#ifndef INIT_HPP
#define INIT_HPP
#include "Window.hpp"

namespace lldb_frontend {
  class Init {
    private:
      static bool isGlfwInit, isGladInit;
      static std::string m_IniFilename;
    private:
      static void err_callback(int error, const char* description);

    public:
      static void InitGlad();
      static void InitGlfw();

      static void InitImGui(const Window&);
      static void DeinitImGui();

      static void TerminateGlfw();
  };
}

#endif
