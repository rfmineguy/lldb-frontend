#ifndef FILE_HEIRARCHY_HPP
#define FILE_HEIRARCHY_HPP
#include <map>
#include <string>
#include <filesystem>
#include <optional>
#include <vector>
#include "FileContext.hpp"

class FileHeirarchy {
  public:
    enum class HeirarchyElementType {
      FILE, FOLDER
    };
    struct HeirarchyElement {
      HeirarchyElementType type;
      std::filesystem::path local_path;
      std::filesystem::path full_path;
      std::filesystem::path relative_path;
      std::string full_path_string;
      const char* c_str;

      std::map<std::string, HeirarchyElement*> children;

      std::optional<std::vector<Line>> lines;

      HeirarchyElement(
          const std::string& local_path,
          const std::string& full_path,
          HeirarchyElementType type = HeirarchyElementType::FOLDER
      );

      public:
        bool LoadFromDisk();
    };
  public:
    FileHeirarchy();
    ~FileHeirarchy();
    void SetWorkingDir(const std::string& workingdir);
    void AddFile(const std::string& dir, const std::string& filename);
    void Print() const;
    HeirarchyElement* GetRoot() const;
    HeirarchyElement* GetElementByLocalPath(const std::string& localpath);

  private:
    void Free(HeirarchyElement* root);
    HeirarchyElement* GetElementByLocalPathRec(HeirarchyElement* root, const std::string& localpath) const;
    void PrintRec(const HeirarchyElement* root, int depth = 0) const;

  private:
    HeirarchyElement* mainRoot;
};

#endif
