#ifndef FILE_CONTEXT_HPP
#define FILE_CONTEXT_HPP
#include <string>
#include <vector>

struct Line {
  std::string line;
  bool bp;
};

struct FileContext {
  std::string filename;
  std::vector<Line> lines;
};

#endif
