#ifndef IMGUI_LAYER_HPP
#define IMGUI_LAYER_HPP
#include "FileHeirarchy.hpp"
struct Window;

class ImGuiLayer {
  public:
    ImGuiLayer();
    ~ImGuiLayer();
    void Begin(Window*);
    void End();
    void BeginDockspace();
    void EndDockspace();
    void Draw();

  private:
    void DrawDebugWindow();
    void DrawCodeWindow();
    bool ShowHeirarchyItem(const FileHeirarchy::HeirarchyElement*);
    void FileHeirarchyRecursive(const FileHeirarchy::HeirarchyElement*);
    void DrawFileBrowser();
  private:
    Window* window_ref;
    FileHeirarchy fh;
};

#endif
