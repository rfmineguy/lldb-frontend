#include "FileHeirarchy.hpp"
#include <iostream>

FileHeirarchy::FileHeirarchy(): mainRoot(new HeirarchyElement("/")) {}
FileHeirarchy::~FileHeirarchy() {
  Free(mainRoot);
}

void FileHeirarchy::Free(HeirarchyElement* element) {
  for (auto& [key, child] : element->children) {
    Free(child);
  }
  delete element;
}

void FileHeirarchy::AddFile(const std::string& dir, const std::string& filename) {
  FileHeirarchy::HeirarchyElement* current = mainRoot;
  auto full_path = std::filesystem::path(dir) / filename;

  auto it = full_path.begin();
  for (; it != full_path.end(); ++it) {
      std::string part = it->string();
      if (part.empty() || part == "/") continue;

      // Last part is file
      bool is_last = std::next(it) == full_path.end();
      bool is_file = is_last; // assume file if it's the end

      auto& children = current->children;
      if (children.find(part) == children.end()) {
          children[part] = new FileHeirarchy::HeirarchyElement(part, is_file ? HeirarchyElementType::FILE : HeirarchyElementType::FOLDER);
      }

      current = children[part];
  }

  std::cout << "Added '" << dir << "' , '" << filename << "'" << std::endl;
}
}
