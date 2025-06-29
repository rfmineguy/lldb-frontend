#ifndef ARGS_HPP
#define ARGS_HPP
#include <argparse/argparse.hpp>

namespace lldb_frontend {
  class Args {
    private:
      static argparse::ArgumentParser parser;
    public:
      static void SetupOptions();
      static void Parse(int argc, char** argv);
      template<typename T>
      static std::optional<T> Get(const std::string& name) {
        return parser.present<T>(name);
      }
  };
}

#endif
