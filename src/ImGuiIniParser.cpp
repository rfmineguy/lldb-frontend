#include "ImGuiIniParser.hpp"

bool ImGuiIniParser::LoadFile(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    std::string currentSection;

    while (std::getline(file, line))
    {
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        if (line.empty()) continue;

        if (line.front() == '[' && line.back() == ']')
        {
            currentSection = line.substr(1, line.size() - 2);
            sections[currentSection] = IniBlock{};
        }
        else if (std::isspace(line[0]))
        {
            // Indented line: treat as raw nested structure
            if (!currentSection.empty())
                sections[currentSection].raw_lines.push_back(line);
        }
        else
        {
            size_t eq = line.find('=');
            if (eq != std::string::npos)
            {
                std::string key = line.substr(0, eq);
                std::string value = line.substr(eq + 1);
                if (!currentSection.empty())
                    sections[currentSection].keys[key] = value;
            }
        }
    }

    return true;
}

const IniBlock* ImGuiIniParser::GetSection(const std::string& section) const
{
    auto it = sections.find(section);
    if (it != sections.end()) return &it->second;
    return nullptr;
}

std::string ImGuiIniParser::GetValue(const std::string& section, const std::string& key, const std::string& fallback) const
{
    auto sec = GetSection(section);
    if (sec)
    {
        auto it = sec->keys.find(key);
        if (it != sec->keys.end())
            return it->second;
    }
    return fallback;
}