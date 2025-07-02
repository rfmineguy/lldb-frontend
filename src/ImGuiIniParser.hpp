#pragma once
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>

struct IniBlock
{
    std::map<std::string, std::string> keys;
    std::vector<std::string> raw_lines; // for nested lines like DockNode trees
};

class ImGuiIniParser
{
public:
    std::map<std::string, IniBlock> sections;

    bool LoadFile(const std::string& path);

    const IniBlock* GetSection(const std::string& section) const;

    std::string GetValue(const std::string& section, const std::string& key, const std::string& fallback = "") const;
};