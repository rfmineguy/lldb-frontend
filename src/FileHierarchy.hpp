#ifndef FILE_HEIRARCHY_HPP
#define FILE_HEIRARCHY_HPP
#include <map>
#include <string>
#include <filesystem>
#include <optional>
#include <vector>
#include "FileContext.hpp"

class FileHierarchy {
  public:
    enum class TreeNodeType {
      FILE,
      FOLDER,
      SYMLINK,
      BLOCK_DEVICE_FILE,
      CHARACTER_DEVICE_FILE,
      FIFO,
      SOCKET,
      COUNT = 7
    };

    struct TreeNode
    {
      TreeNode* parent_node = 0;
      std::string name;
      std::filesystem::path path;
      std::optional<std::vector<Line>> lines;
      std::map<std::string, TreeNode> children;
      bool shouldSwitch;

      std::filesystem::path GetPath() const;

      std::pair<std::filesystem::path, TreeNode*> LookaheadPath();

      static std::pair<std::filesystem::path, TreeNode*> LookaheadPathRec(TreeNode& node);

      TreeNode *Insert(const std::vector<std::string> &parts, size_t index = 0);

      void Print(int depth = 0) const;

      bool LoadFromDisk();
    };

    static TreeNodeType GetTypeFromNode(const TreeNode& node);
    static TreeNodeType GetOSPathType(const std::filesystem::path &path);
  
  public:
    void AddFile(const std::filesystem::path& path);
    TreeNode& GetRoot();
    TreeNode* GetElementByFilename(const std::string& filename);
    TreeNode* GetElementByLocalPath(const std::filesystem::path& localpath);
    void ComputeTree();

  private:
    TreeNode* GetElementByFilenameRec(TreeNode& node, const std::string& filename) const;
    TreeNode* GetElementByLocalPathRec(TreeNode& node, const std::filesystem::path& localpath) const;

  private:
    std::vector<std::filesystem::path> paths;
    TreeNode root;
};

#endif
