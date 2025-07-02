#ifndef TEXTURE_HPP
#define TEXTURE_HPP
#include <string>
#include <glad/gl.h>
struct ImVec2;

class Texture {
  public:
    Texture(const std::string& local_path);
    ~Texture();
    unsigned int GetTextureId() const;
    int GetWidth() const;
    int GetHeight() const;
    int GetChannels() const;
    ImVec2 GetImGuiSizeScaled(float scaleFactor) const;
  private:
    void Load();
  private:
    bool isLoaded;
    unsigned int textureId;
    int width, height, channels;
    std::filesystem::path full_path;
    std::string local_path;
};

#endif
