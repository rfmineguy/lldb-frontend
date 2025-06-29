#ifndef IMGUI_LAYER_HPP
#define IMGUI_LAYER_HPP
#include "FileHeirarchy.hpp"
#include "FileContext.hpp"
#include <unordered_map>
#include <vector>
struct Window;
struct ImGuiInputTextCallbackData;
class LLDBDebugger;

class ImGuiLayer {
  public:
    ImGuiLayer(LLDBDebugger& debugger);
    ~ImGuiLayer();
    void Begin(Window*);
    void End();
    void BeginDockspace();
    void EndDockspace();
    void Draw();
    LLDBDebugger& GetDebugger();
    void DrawFilesNotFoundModal();

  private:
    bool LoadFile(const std::string&);

  private:
    void DrawDebugWindow();
    void DrawCodeWindow();
    void DrawStackTraceWindow();
    void DrawControlsWindow();
    void DrawBreakpointsWindow();
    void DrawLLDBCommandWindow();

    bool ShowHeirarchyItem(FileHeirarchy::HeirarchyElement*);
    void FileHeirarchyRecursive(FileHeirarchy::HeirarchyElement*);
    void DrawFileBrowser();

  private:
    void DrawCodeFile(FileHeirarchy::HeirarchyElement&);

  private:
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

  private:
    LLDBDebugger& debugger;
    Window* window_ref;
    FileHeirarchy fh;
    std::unordered_map<std::string, FileContext> fileContentsMap;
    std::vector<FileHeirarchy::HeirarchyElement*> openFiles;
    bool m_FilesNotFoundModal_open = false;
    std::vector<const FileHeirarchy::HeirarchyElement*> m_FilesNotFoundModal_files;
};

#endif
