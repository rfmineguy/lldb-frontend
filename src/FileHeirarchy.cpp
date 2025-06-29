#include "FileHeirarchy.hpp"
#include "Logger.hpp"
#include <iostream>
#include <fstream>

// HeirarchyElement
FileHeirarchy::HeirarchyElement::HeirarchyElement(
    const std::string& local_path,
    const std::string& full_path,
    HeirarchyElementType type
):
  local_path(local_path),
  full_path(full_path),
  full_path_string(full_path),
  type(type),
  c_str(full_path_string.c_str()),
  lines(std::vector<Line>{0, Line{}}) {}

bool FileHeirarchy::HeirarchyElement::LoadFromDisk() {
  Logger::ScopedGroup g("HeirarchyElement::LoadFromDisk");
  auto& lines_ref = lines.value();
  std::ifstream f(full_path);
  if (!f.is_open()) {
    Logger::Crit("Failed to load {} from disk", full_path.string());
    return false;
  }
  std::string line;
  while (std::getline(f, line)) {
    lines_ref.push_back({.line = line, .bp = false});
  }
  f.close();
  Logger::Info("Loaded {} from disk", full_path.string());
  return true;
}

// FileHeirarchy
FileHeirarchy::FileHeirarchy(): mainRoot(new HeirarchyElement("/", "/")) {}
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
          children[part] = new FileHeirarchy::HeirarchyElement(part, full_path.string(), is_file ? HeirarchyElementType::FILE : HeirarchyElementType::FOLDER);
      }

      current = children[part];
  }

  Logger::Info("Added '{}' , '{}'", dir, filename);
}

void FileHeirarchy::PrintRec(const FileHeirarchy::HeirarchyElement* root, int depth) const {
  if (!root) return;
  
  // Is it a folder or a file?
  switch (root->type) {
    case FileHeirarchy::HeirarchyElementType::FOLDER:
      Logger::Info("{}{}", std::string(depth * 2, ' '), root->local_path.filename().string());
      break;
    case FileHeirarchy::HeirarchyElementType::FILE:
      Logger::Info("{}{}", std::string(depth * 2, ' '), root->local_path.filename().string());
      break;
  }

  // Now we need to process the children
  for (auto& [key, child] : root->children) {
    PrintRec(child, depth + 1);
  }
}

void FileHeirarchy::Print() const {
  Logger::BeginGroup("FileHeirarchy::Print");
  PrintRec(mainRoot, 0);
  Logger::EndGroup();
}

FileHeirarchy::HeirarchyElement* FileHeirarchy::GetRoot() const {
  return mainRoot;
}
