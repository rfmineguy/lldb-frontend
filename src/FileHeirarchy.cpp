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
}
