#include <cxxopts.hpp>
#include <fmt/base.h>
#include <random>
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
  opt_adder(
      "c,num_ccaps",
      "The minimum number of coupling capacitances each net will have",
      cxxopts::value<std::size_t>());
  opt_adder(
      "s,seed",
      "The seed for the random number generator",
      cxxopts::value<unsigned int>());
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
  if (result.count("num_ccaps") != 0) {
    config.min_num_ccaps = result["num_ccaps"].as<std::size_t>();
  }
  if (result.count("seed") != 0) {
    config.seed = result["seed"].as<unsigned int>();
  } else {
    config.seed = std::random_device{}();
  }
  config.init_rand();

  std::jthread block_verilog(write_block_verilog, std::ref(config));
  std::jthread top_verilog(write_top_verilog, std::ref(config));
  // we can't generate block and top SPEF in parallel, because it messes up the
  // random number generator
  std::jthread spef([&config]() {
    write_block_spef(std::ref(config));
    write_top_spef(std::ref(config));
  });

  return 0;
}
