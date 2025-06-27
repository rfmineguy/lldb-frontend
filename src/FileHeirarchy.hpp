#ifndef FILE_HEIRARCHY_HPP
#define FILE_HEIRARCHY_HPP
#include <map>
#include <string>
#include <filesystem>

class FileHeirarchy {
  public:
    enum class HeirarchyElementType {
      FILE, FOLDER
    };
    struct HeirarchyElement {
      HeirarchyElementType type;
      std::filesystem::path path;

      std::map<std::string, HeirarchyElement*> children;

      HeirarchyElement(const std::string& path, HeirarchyElementType type = HeirarchyElementType::FOLDER):
        path(path),
        type(type) {}
    };
  public:
    FileHeirarchy();
    ~FileHeirarchy();
    void AddFile(const std::string& dir, const std::string& filename);
    void Print() const;
    HeirarchyElement* GetRoot() const;

  private:
    void Free(HeirarchyElement* root);
    void PrintRec(HeirarchyElement* root, int depth = 0) const;

  private:
    HeirarchyElement* mainRoot;
};

#endif
