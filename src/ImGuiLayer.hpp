#ifndef IMGUI_LAYER_HPP
#define IMGUI_LAYER_HPP
#include "FileHierarchy.hpp"
#include "FileContext.hpp"
#include <unordered_map>
#include <vector>
#include <queue>
struct Window;
struct ImGuiInputTextCallbackData;
class LLDBDebugger;

class ImGuiLayer {
  friend LLDBDebugger;
  public:
    ImGuiLayer(LLDBDebugger& debugger);
    ~ImGuiLayer();
    void Begin(Window*);
    void End();
    void BeginDockspace();
    void EndDockspace();
    void Draw();
    LLDBDebugger& GetDebugger();
    FileHierarchy& GetFileHierarchy();
    void DrawFilesNotFoundModal();
    void SwitchToCodeFile(const std::string&);
  
  protected:
    bool FrontendLoadFile(FileHierarchy::TreeNode&);

  private:
    bool LoadFile(FileHierarchy::TreeNode&);

  private:
    void DrawDebugWindow();
    void DrawCodeWindow();
    void DrawStackTraceWindow();
    void DrawControlsWindow();
    void DrawBreakpointsWindow();
    void DrawLLDBCommandWindow();

    bool ShowHierarchyItem(FileHierarchy::TreeNode&, const std::filesystem::path&, const std::filesystem::path&);
    void FileHierarchyRecursive(const std::filesystem::path&, FileHierarchy::TreeNode&);
    void DrawFileBrowser();

  private:
    void DrawCodeFile(FileHierarchy::TreeNode&);

  private:
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);

  private:
    LLDBDebugger& debugger;
    Window* window_ref;
    FileHierarchy fh;
    std::vector<FileHierarchy::TreeNode*> openFiles;
    bool m_FilesNotFoundModal_open = false;
    std::vector<const FileHierarchy::TreeNode*> m_FilesNotFoundModal_files;
};

#endif
