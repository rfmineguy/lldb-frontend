#include "Texture.hpp"
#include "Logger.hpp"
#include <imgui.h>
#include <stb_image.h>
#include <filesystem>

Texture::Texture(const std::string& local_path)
  :local_path(local_path) {
  // run logic for loading the texture
  Load();
}

Texture::~Texture() {
  // nothing to delete if not loaded
  if (!isLoaded) return;

  // free the texture memory
  glDeleteTextures(1, &textureId);
  Logger::Info("Deleted texture {}", textureId);
}

unsigned int Texture::GetTextureId() const {
  return textureId;
}

int Texture::GetWidth() const {
  return width;
}

int Texture::GetHeight() const {
  return height;
}

int Texture::GetChannels() const {
  return channels;
}

ImVec2 Texture::GetImGuiSizeScaled(float scaleFactor) const {
  return ImVec2{(float)width * scaleFactor, (float)height * scaleFactor};
}


void Texture::Load() {
  Logger::ScopedGroup g("TextureLoad");
  if (isLoaded) return;
  // allocate and setup gpu texture
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // get full path
  auto full = std::filesystem::absolute(local_path);
  Logger::Info("Full path: {}", full.string());

  // stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(full.string().c_str(), &width, &height, &channels, 0);
  if (!data) {
    Logger::Crit("Failed to load image path {}", full.string());
    isLoaded = false; // redundant, but explicit
    stbi_image_free(data);
    return;
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);

  isLoaded = true;
  Logger::Info("Loaded image path {}", full.string());
}
