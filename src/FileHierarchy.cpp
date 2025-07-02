#include "FileHierarchy.hpp"
#include "Logger.hpp"
#include "Util.hpp"
#include <iostream>
#include <fstream>
#if defined(_WIN32)
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#elif defined(__unix__) || defined(__APPLE__)
#include <sys/stat.h>
#include <unistd.h>
#endif

std::filesystem::path FileHierarchy::TreeNode::GetPath() const {
  static std::string path_sep_string(1, char(std::filesystem::path::preferred_separator));
  std::string path_string(name);
  auto current_parent_node = parent_node;
  while (current_parent_node) {
#if defined(_WIN32)
    if (!(current_parent_node->name.empty() || current_parent_node->name == path_sep_string)) {
#else
    if (!(current_parent_node->name == path_sep_string)) {
#endif
      if (path_string != path_sep_string)
        path_string.insert(path_string.begin(), path_sep_string.begin(), path_sep_string.end());
      path_string.insert(path_string.begin(), current_parent_node->name.begin(), current_parent_node->name.end());
    }
    current_parent_node = current_parent_node->parent_node;
  }
  if (GetOSPathType(std::filesystem::path(path_string)) == FileHierarchy::TreeNodeType::FOLDER) {
    if (path_string[path_string.size() - 1] != path_sep_string[0]) {
      path_string += path_sep_string;
    }
  }
  else if (!children.empty())
    path_string += path_sep_string;
  return std::filesystem::path(path_string);
}

std::pair<std::filesystem::path, FileHierarchy::TreeNode*> FileHierarchy::TreeNode::LookaheadPath() {
  return LookaheadPathRec(*this);
}

std::pair<std::filesystem::path, FileHierarchy::TreeNode*> FileHierarchy::TreeNode::LookaheadPathRec(TreeNode& node) {
  int folder_count = 0;
  int file_count = 0;
  for (auto& [key, child] : node.children) {
    auto child_type = GetTypeFromNode(child);
    switch (child_type) {
      case TreeNodeType::FILE:
        file_count++;
        break;
      case TreeNodeType::FOLDER:
        folder_count++;
        break;
      default:
        break;
    }
  }
  if (file_count == 0 && folder_count == 1) {
    return LookaheadPathRec(node.children.begin()->second);
  }
  else {
    return {
      node.GetPath(),
      &node
    };
  }
}

FileHierarchy::TreeNode* FileHierarchy::TreeNode::Insert(const std::vector<std::string> &parts, size_t index) {
  if (index >= parts.size()) {
    return this;
  }

  const std::string &part = parts[index];

  auto iter = children.find(part);
  if (iter == children.end()) {
    TreeNode node;
    node.name = part;
    node.parent_node = this;
    node.path = node.GetPath();
    iter = children.insert({part, node}).first;
  }

  return iter->second.Insert(parts, index + 1);
}

void FileHierarchy::TreeNode::Print(int depth) const {
  if (!name.empty()) {
    for (int i = 0; i < depth; ++i)
    {
        std::cout << "   ";
    }
    std::cout << name << "\t| " << path.string() << std::endl;
  }

  for (const auto &child : children) {
    child.second.Print(depth + 1);
  }
}

bool FileHierarchy::TreeNode::LoadFromDisk() {
  Logger::ScopedGroup g("TreeNode::LoadFromDisk");
  if (!lines.has_value()) {
    lines = std::vector<Line>{};
    if (Util::ReadFileLinesIntoVector(path.string(), lines.value())) {
      Logger::Info("Loaded {} from disk", path.string());
    }
  }
  return true;
}

FileHierarchy::TreeNodeType FileHierarchy::GetTypeFromNode(const TreeNode& node) {
  auto type = GetOSPathType(node.path);
  if (!node.children.empty() && type == FileHierarchy::TreeNodeType::FILE)
    return FileHierarchy::TreeNodeType::FOLDER;
  return type;
}

FileHierarchy::TreeNodeType FileHierarchy::GetOSPathType(const std::filesystem::path &path)
{
#if defined(_WIN32)
  const DWORD attributes = GetFileAttributesW(path.wstring().c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES)
  {
    return FileHierarchy::TreeNodeType::FILE;
  }
  if (attributes & FILE_ATTRIBUTE_REPARSE_POINT)
  {
    return FileHierarchy::TreeNodeType::SYMLINK;
  }
  if (attributes & FILE_ATTRIBUTE_DIRECTORY)
  {
    return FileHierarchy::TreeNodeType::FOLDER;
  }
  return FileHierarchy::TreeNodeType::FILE;

#elif defined(__unix__) || defined(__APPLE__)
  struct stat s;
  if (lstat(path.c_str(), &s) != 0)
  {
    return FileHierarchy::TreeNodeType::FILE;
  }
  if (S_ISREG(s.st_mode))
  {
    return FileHierarchy::TreeNodeType::FILE;
  }
  if (S_ISDIR(s.st_mode))
  {
    return FileHierarchy::TreeNodeType::FOLDER;
  }
  if (S_ISLNK(s.st_mode))
  {
    return FileHierarchy::TreeNodeType::SYMLINK;
  }
  if (S_ISBLK(s.st_mode))
  {
    return FileHierarchy::TreeNodeType::BLOCK_DEVICE_FILE;
  }
  if (S_ISCHR(s.st_mode))
  {
    return FileHierarchy::TreeNodeType::CHARACTER_DEVICE_FILE;
  }
  if (S_ISFIFO(s.st_mode))
  {
    return FileHierarchy::TreeNodeType::FIFO;
  }
  if (S_ISSOCK(s.st_mode))
  {
    return FileHierarchy::TreeNodeType::SOCKET;
  }
  return FileHierarchy::TreeNodeType::FILE;

#else
  return FileHierarchy::TreeNodeType::FILE;
#endif
}

void FileHierarchy::AddFile(const std::filesystem::path& path) {
  paths.push_back(path);
  Logger::Info("Added '{}'", path.string());
}

FileHierarchy::TreeNode& FileHierarchy::GetRoot() {
  return root;
}

FileHierarchy::TreeNode* FileHierarchy::GetElementByFilename(const std::string& filename) {
  for (auto& [key, child] : root.children) {
      auto found = GetElementByFilenameRec(child, filename);
      if (found) return found;
  }
  return nullptr;
}

FileHierarchy::TreeNode* FileHierarchy::GetElementByLocalPath(const std::filesystem::path& localpath) {
  for (auto& [key, child] : root.children) {
      auto found = GetElementByLocalPathRec(child, localpath);
      if (found) return found;
  }
  return nullptr;
}

std::vector<std::string> PathToParts(const std::filesystem::path &path)
{
    std::vector<std::string> parts;

    for (const auto &part : path)
    {
        parts.push_back(part.string());
    }

    return parts;
}

void FileHierarchy::ComputeTree()
{
  for (const auto &path : paths)
  {
    std::vector<std::string> parts = PathToParts(path);
    root.Insert(parts);
  }
  root.Print();
}

FileHierarchy::TreeNode* FileHierarchy::GetElementByFilenameRec(TreeNode& node, const std::string& filename) const {
  if (node.name == filename) return &node;
  for (auto& [key, child] : node.children) {
      auto result = GetElementByFilenameRec(child, filename);
      if (result) return result;
  }
  return nullptr;
}

FileHierarchy::TreeNode* FileHierarchy::GetElementByLocalPathRec(TreeNode& node, const std::filesystem::path& localpath) const {
  if (node.GetPath() == localpath) return &node;
  for (auto& [key, child] : node.children) {
      auto result = GetElementByLocalPathRec(child, localpath);
      if (result) return result;
  }
  return nullptr;
}
