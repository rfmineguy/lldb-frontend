#ifndef FILE_CONTEXT_HPP
#define FILE_CONTEXT_HPP
#include <lldb/API/LLDB.h>
#include <string>
#include <vector>

struct Line {
  std::string line;
  bool bp;
  lldb::break_id_t bp_id = LLDB_INVALID_BREAK_ID;
};

struct FileContext {
  std::string filename;
  std::vector<Line> lines;
};

#endif
