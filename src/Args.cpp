#include "Args.hpp"

namespace lldb_frontend {
  argparse::ArgumentParser Args::parser = argparse::ArgumentParser();

  void Args::SetupOptions() {
    parser.add_argument("--executable")
      .help("The program you wish to debug");
    parser.add_argument("--autoexec")
      .help("Script file containing autoexec instructions");
  }

  bool Args::Parse(int argc, char **argv) {
    try {
      parser.parse_args(argc, argv);
      return true;
    } catch (const std::exception& err) {
      std::cerr << err.what() << std::endl;
      std::cerr << parser;
      return false;
    }
  }

  void Args::ShowHelp() {
    std::cout << parser.help().str() << std::endl;
  }
}
