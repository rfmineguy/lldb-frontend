#include "Args.hpp"

namespace lldb_frontend {
  argparse::ArgumentParser Args::parser = argparse::ArgumentParser();

  void Args::SetupOptions() {
    parser.add_argument("-e", "--executable")
      .help("The program you wish to debug");
  }

  void Args::Parse(int argc, char **argv) {
    try {
      parser.parse_args(argc, argv);
    } catch (const std::exception& err) {
      std::cerr << err.what() << std::endl;
      std::cerr << parser;
      exit(1);
    }
  }
}
