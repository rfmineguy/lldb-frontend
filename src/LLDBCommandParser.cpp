#include "LLDBCommandParser.hpp"
#include "Logger.hpp"
#include <sstream>
#include <string> 

LLDB_CommandParser::LLDB_CommandParser() {}
LLDB_CommandParser::~LLDB_CommandParser() {}

LLDB_CommandParser::ParsedCommand LLDB_CommandParser::Parse(const std::string& command) {
  std::vector<std::string> split = SplitBySpaces(command);
  if (split.size() == 0) return LLDB_CommandParser::ParsedCommand{.type = ParsedCommandType::EMPTY};

  // Parse short form first
  //  b main.cpp:3
  if (split.at(0) == "b") {
    if (split.size() == 1) return LLDB_CommandParser::Invalid("Incomplete breakpoint command");
    auto& where = split.at(1);
    size_t colon = where.find(":");
    if (colon != std::string::npos) { // b <file>:<line>
      std::string file = where.substr(0, colon);
      std::string line = where.substr(colon + 1);
      try {
        int line_int = std::stoi(std::string(line));
        return ParsedCommand{.type = ParsedCommandType::BREAKPOINT_FILE_LINE, .command = BPFileLine{file, line_int}};
      } catch (std::invalid_argument e) {
        return LLDB_CommandParser::Invalid("Breakpoint line '{}' not an integer", line);
      } catch (std::out_of_range e) {
        return LLDB_CommandParser::Invalid("Breakpoint line '{}' out of range", line);
      }
    }
    else { // b <symbol>
      return ParsedCommand{.type = ParsedCommandType::BREAKPOINT_SYMBOL, .command = BPSymbol{where}};
    }
  }
  if (split.size() == 1 && split.at(0) == "n") {
    return ParsedCommand{.type = ParsedCommandType::NEXT};
  }
  if (split.size() == 1 && split.at(0) == "s") {
    return ParsedCommand{.type = ParsedCommandType::STEP};
  }
  if (split.size() == 1 && split.at(0) == "c") {
    return ParsedCommand{.type = ParsedCommandType::CONTINUE};
  }
  if (split.size() == 1 && split.at(0) == "r") {
    return ParsedCommand{.type = ParsedCommandType::RUN};
  }
  return LLDB_CommandParser::Invalid("{} not valid", command);
}

std::vector<std::string> LLDB_CommandParser::SplitBySpaces(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s); // Create an input string stream from the string
    std::string token;

    // Extract tokens until the end of the stream
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}
