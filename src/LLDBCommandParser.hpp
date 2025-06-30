#ifndef LLDB_COMMAND_PARSER_HPP
#define LLDB_COMMAND_PARSER_HPP
#include <string_view>
#include <vector>
#include <variant>
#include <fmt/core.h>

class LLDB_CommandParser {
public:
  enum class ParsedCommandType : int {
    EMPTY = 0, INVALID,
    BREAKPOINT_FILE_LINE, BREAKPOINT_SYMBOL,
    RUN, CONTINUE, STEP, NEXT,
  };
  struct InvalidCmd {
    std::string message;
  };
  struct BPFileLine {
    std::string file;
    int line;
  };
  struct BPSymbol {
    std::string symbol;
  };
  struct ParsedCommand {
    ParsedCommandType type;
    std::variant<
      std::monostate,
      InvalidCmd,
      BPFileLine,
      BPSymbol
    > command;
  };
public:
  LLDB_CommandParser();
  ~LLDB_CommandParser();
  ParsedCommand Parse(const std::string& command);

protected:
  template <typename ...Args>
  static ParsedCommand Invalid(std::string_view fmt, Args&&... args) {
    return ParsedCommand{.type = ParsedCommandType::INVALID,
      .command = InvalidCmd(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args)...)))
    };
  }

private:
  std::vector<std::string> SplitBySpaces(const std::string& s);
};

#endif
