#ifndef IMGUI_LAYER_HPP
#define IMGUI_LAYER_HPP
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
    Window* window_ref;
};

#endif
