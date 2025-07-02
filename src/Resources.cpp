#include "Resources.hpp"
#include "Logger.hpp"

namespace lldb_frontend {
  TextureMap Resources::textures = TextureMap();

  Resources::Resources() {}
  Resources::~Resources() {}
  void Resources::LoadAll() {
    Logger::ScopedGroup g("Resources::LoadAll");
    textures.emplace("folder", "resources/Folder.png");
    textures.emplace("file",   "resources/File.png");
  }
  const Texture* Resources::GetTexture(const std::string &name) {
    if (!textures.contains(name)) return nullptr;
    return &textures.at(name);
  }
}
