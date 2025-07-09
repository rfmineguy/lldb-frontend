#include "ThemeListener.hpp"
#include "Util.hpp"
#include <windows.h>
#include <chrono>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

static WNDPROC g_OriginalWndProc = nullptr;
static void (*g_ThemeChangeCallback)() = nullptr;

static LRESULT CALLBACK ThemeAwareWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static bool lastThemeDark = Util::GetSystemTheme() == Util::SystemTheme::DARK;

    if ((msg == WM_THEMECHANGED || msg == WM_SETTINGCHANGE) && g_ThemeChangeCallback) {
        bool currentDark = Util::GetSystemTheme() == Util::SystemTheme::DARK;
        if (currentDark != lastThemeDark) {
            lastThemeDark = currentDark;
            g_ThemeChangeCallback();
        }
    }

    return CallWindowProc(g_OriginalWndProc, hwnd, msg, wParam, lParam);
}

void RegisterThemeChangeObserver(void (*callback)()) {
    GLFWwindow* curr = glfwGetCurrentContext();
    HWND hwnd = glfwGetWin32Window(curr);
    g_ThemeChangeCallback = callback;
    g_OriginalWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ThemeAwareWndProc);
}
