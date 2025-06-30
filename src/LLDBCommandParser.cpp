#include "LLDBCommandParser.hpp"
#include "Logger.hpp"
#include <sstream>
#include <string> 

LLDB_CommandParser::LLDB_CommandParser() {}
LLDB_CommandParser::~LLDB_CommandParser() {}

LLDB_CommandParser::ParsedCommand LLDB_CommandParser::Parse(const std::string& command) {
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
