#ifdef WRITE_COMPRESSED
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <ostream>
#else
#include <fstream>
#endif

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string>

#include "design_config.hpp"

// forward declarations
template <typename OSTREAM>
void write_wires(OSTREAM &os, design_config const &config);
template <typename OSTREAM>
void write_cells(OSTREAM &os, design_config const &config);
template <typename OSTREAM>
void write_wrapped(
    OSTREAM &os,
    std::string const &str,
    std::size_t first_indent,
    std::size_t rest_indent,
    std::size_t column);
template <typename OSTREAM>
void write_wrapped_too_long_indent(
    OSTREAM &os,
    std::string const &str,
    std::size_t first_indent,
    std::size_t rest_indent);

void write_block_verilog(design_config const &config) {
#ifdef WRITE_COMPRESSED
  std::string filename{config.block_name + ".v.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream os(&buf);
#else
  std::string filename{config.block_name + ".v"};
  std::ofstream os(filename);
#endif
  fmt::println(os, "module {}(A);", config.block_name);
  fmt::println(os, "  input A;");
  write_wires(os, config);
  write_cells(os, config);
  fmt::println(os, "endmodule");
}

void write_top_verilog(design_config const &config) {
#ifdef WRITE_COMPRESSED
  std::string filename{config.top_name + ".v.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream os(&buf);
#else
  std::string filename{config.top_name + ".v"};
  std::ofstream os(filename);
#endif
  {
    std::string module_line = fmt::format("module {}(A1", config.top_name);
    for (std::size_t block_idx = 2; block_idx <= config.num_blocks; ++block_idx) {
      module_line += fmt::format(", A{}", block_idx);
    }
    module_line += ");";
    write_wrapped(os, module_line, 0, 2, config.num_cols);
  }

  {
    std::string input_line = fmt::format("input A1");
    for (std::size_t block_idx = 2; block_idx <= config.num_blocks; ++block_idx) {
      input_line += fmt::format(", A{}", block_idx);
    }
    input_line += ";";
    write_wrapped(os, input_line, 2, 2, config.num_cols);
  }

  for (std::size_t block_idx = 0; block_idx < config.num_blocks; ++block_idx) {
    fmt::println(
        os,
        "  {} {}{}(.A(A{}));",
        config.block_name,
        config.block_prefix,
        block_idx + 1,
        block_idx + 1);
  }
  fmt::println(os, "endmodule");
}

template <typename OSTREAM>
void write_wires(OSTREAM &os, design_config const &config) {
  std::string wire_line = fmt::format("wire {}1", config.net_prefix);
  for (std::size_t net_idx = 2; net_idx < config.num_nets; ++net_idx) {
    wire_line += fmt::format(", {}{}", config.net_prefix, net_idx);
  }
  wire_line += ";";
  write_wrapped(os, wire_line, 2, 2, config.num_cols);
}

template <typename OSTREAM>
void write_cells(OSTREAM &os, design_config const &config) {
  // the first cell is a special case, since it connects to the input port
  fmt::println(
      os,
      "  {} {}{}(.{}(A), .{}({}{}));",
      config.lib_cell_name,
      config.cell_prefix,
      1,
      config.lib_cell_inp_pin,
      config.lib_cell_out_pin,
      config.net_prefix,
      1);
  for (std::size_t net_idx = 2; net_idx < config.num_nets; ++net_idx) {
    fmt::println(
        os,
        "  {} {}{}(.{}({}{}), .{}({}{}));",
        config.lib_cell_name,
        config.cell_prefix,
        net_idx,
        config.lib_cell_inp_pin,
        config.net_prefix,
        net_idx / 2,
        config.lib_cell_out_pin,
        config.net_prefix,
        net_idx);
  }
  // leaf cells
  for (std::size_t net_idx = config.num_nets; net_idx < 2 * config.num_nets; ++net_idx) {
    fmt::println(
        os,
        "  {} {}{}(.{}({}{}));",
        config.lib_leaf_cell_name,
        config.cell_prefix,
        net_idx,
        config.lib_leaf_cell_d_pin,
        config.net_prefix,
        net_idx / 2);
  }
}

template <typename OSTREAM>
void write_wrapped(
    OSTREAM &os,
    std::string const &str,
    std::size_t first_indent,
    std::size_t rest_indent,
    std::size_t column) {
  std::string first_indent_str(first_indent, ' ');
  std::string rest_indent_str(rest_indent, ' ');

  if (first_indent >= column || rest_indent >= column) {
    write_wrapped_too_long_indent(os, str, first_indent, rest_indent);
    return;
  }

  std::size_t left = 0;
  while (left < str.size()) {
    std::size_t right =
        left + column - (left == 0 ? first_indent : rest_indent);
    if (right >= str.size()) {
      break;
    }
    while (right > left && str[right] != ' ') {
      --right;
    }
    if (right == left) {
      while (right < str.size() && str[right] != ' ') {
        ++right;
      }
    }
    if (right >= str.size()) {
      break;
    }
    fmt::println(
        os,
        "{}{}",
        left == 0 ? first_indent_str : rest_indent_str,
        str.substr(left, right - left));
    left = right + 1;
  }
  fmt::println(
      os,
      "{}{}",
      left == 0 ? first_indent_str : rest_indent_str,
      str.substr(left, str.size() - left));
}

template <typename OSTREAM>
void write_wrapped_too_long_indent(
    OSTREAM &os,
    std::string const &str,
    std::size_t first_indent,
    std::size_t rest_indent) {
  std::string first_indent_str(first_indent, ' ');
  std::string rest_indent_str(rest_indent, ' ');

  std::size_t left = 0;
  while (left < str.size()) {
    std::size_t right = left;
    while (right < str.size() && str[right] != ' ') {
      ++right;
    }
    fmt::println(
        os,
        "{}{}",
        left == 0 ? first_indent_str : rest_indent_str,
        str.substr(left, right - left));
    left = right + 1;
  }
}
