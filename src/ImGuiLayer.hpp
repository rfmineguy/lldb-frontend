#ifndef IMGUI_LAYER_HPP
#define IMGUI_LAYER_HPP
#include "FileHeirarchy.hpp"
#include <unordered_map>
#include <vector>
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
    bool LoadFile(const std::string&);

  private:
    void DrawDebugWindow();
    void DrawCodeWindow();
    bool ShowHeirarchyItem(const FileHeirarchy::HeirarchyElement*);
    void FileHeirarchyRecursive(const FileHeirarchy::HeirarchyElement*);
    void DrawFileBrowser();
  private:
    Window* window_ref;
    FileHeirarchy fh;
    std::unordered_map<std::string, std::vector<std::string>> fileContentsMap;
};

#endif
