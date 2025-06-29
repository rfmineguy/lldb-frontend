//=== Platform Detection ===//
#if defined(_WIN32)
  #include <windows.h>
  #include <io.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
#elif defined(__linux__)
  #include <unistd.h>
#endif

#ifndef _WIN32
  #include <unistd.h>
#endif

//=== Standard Library ===//
#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <assert.h>

//=== OpenGL & GLFW ===//
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//=== ImGui ===//
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

//=== Third-Party ===//
#include <tinyfiledialogs.h>

//=== Project Headers ===//
#include "Args.hpp"
#include "FileContext.hpp"
#include "FileHeirarchy.hpp"
#include "ImGuiCustomWidgets.hpp"
#include "ImGuiLayer.hpp"
#include "Init.hpp"
#include "LLDBDebugger.hpp"
#include "Logger.hpp"
#include "Util.hpp"
#include "Window.hpp"