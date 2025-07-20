#ifndef ARGS_HPP
#define ARGS_HPP
#include <argparse/argparse.hpp>

namespace lldb_frontend {
  class Args {
    private:
      static argparse::ArgumentParser parser;
    public:
      static void SetupOptions();
      static bool Parse(int argc, char** argv);
      static void ShowHelp();
      static std::vector<std::string> ForwardArgs();
    public:
      template<typename T>
      static std::optional<T> Get(const std::string& name) {
        return parser.present<T>(name);
      }
  };
}

#endif
