#ifndef IMGUI_LAYER_HPP
#define IMGUI_LAYER_HPP
#include "FileHeirarchy.hpp"
#include "FileContext.hpp"
#include <unordered_map>
#include <vector>
struct Window;
struct ImGuiInputTextCallbackData;

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
    void DrawStackTraceWindow();
    void DrawControlsWindow();
    void DrawBreakpointsWindow();
    void DrawLLDBCommandWindow();

    bool ShowHeirarchyItem(const FileHeirarchy::HeirarchyElement*);
    void FileHeirarchyRecursive(const FileHeirarchy::HeirarchyElement*);
    void DrawFileBrowser();

  private:
    void DrawCodeFile(FileContext&);

  private:
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

  private:
    Window* window_ref;
    FileHeirarchy fh;
    std::unordered_map<std::string, FileContext> fileContentsMap;
    std::vector<FileHeirarchy::HeirarchyElement*> openFiles;
};

#endif
