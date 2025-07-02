#ifndef RESOURCES_HPP
#define RESOURCES_HPP
#include "Texture.hpp"
#include <map>
#include <optional>

namespace lldb_frontend {
  using TextureMap = std::map<std::string, Texture>;
  class Resources {
    private:
      Resources();
      ~Resources();
    public:
      // Must be called after glfw and glad are initialized
      static void LoadAll();
      static const Texture* GetTexture(const std::string&);
    private:
      static TextureMap textures;
  };
}

#endif
