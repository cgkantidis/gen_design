#include <cxxopts.hpp>
#include <fmt/base.h>
#include <thread>

#include "design_config.hpp"
#include "gen_spef.hpp"
#include "gen_verilog.hpp"

int main(int argc, char const *const *argv) {
  cxxopts::Options options(
      "gen_design",
      "Generate the necessary verilog and SPEF files for a design");
  auto opt_adder = options.add_options();
  opt_adder(
      "n,num_nets",
      "The number of nets in each block hierarchy",
      cxxopts::value<std::size_t>());
  opt_adder(
      "b,num_blocks",
      "The number of block hierarchies in the top hierarchy",
      cxxopts::value<std::size_t>());
  opt_adder("h,help", "Print this help message");
  auto result = options.parse(argc, argv);

  if (result.count("help") != 0) {
    fmt::println("{}", options.help());
    return 0;
  }

  design_config config;
  if (result.count("num_nets") != 0) {
    config.num_nets = result["num_nets"].as<std::size_t>();
  }
  if (result.count("num_blocks") != 0) {
    config.num_blocks = result["num_blocks"].as<std::size_t>();
  }

  std::jthread block_verilog(write_block_verilog, std::ref(config));
  std::jthread top_verilog(write_top_verilog, std::ref(config));
  std::jthread block_spef(write_block_spef, std::ref(config));
  std::jthread top_spef(write_top_spef, std::ref(config));

  return 0;
}
