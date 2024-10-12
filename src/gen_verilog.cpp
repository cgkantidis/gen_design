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

// 1'000'000 x 4'500 = 4.5B, which is a little larger than 2^32
static constexpr std::size_t num_nets{10};
static constexpr std::size_t num_blocks{2};
static constexpr std::size_t num_cols{80};
static constexpr std::string block_name{"block"};
static constexpr std::string top_name{"top"};
static constexpr std::string block_prefix{"b"};
static constexpr std::string cell_prefix{"u"};
static constexpr std::string net_prefix{"n"};
static constexpr std::string lib_cell_name{"IV"};
static constexpr std::string lib_cell_inp_pin{"A"};
static constexpr std::string lib_cell_out_pin{"Z"};

template <typename OSTREAM>
void write_wires(OSTREAM &os) {
  static constexpr std::size_t net_prefix_sz{net_prefix.size()};
  std::string wire_line;
  std::size_t num_digits = 1;
  std::size_t last_idx = 1;
  for (std::size_t net_idx = 1; net_idx < num_nets; ++net_idx) {
    if (wire_line.empty()) {
      wire_line = std::string("  wire ") + net_prefix + std::to_string(net_idx);
      continue;
    }

    if (net_idx >= 10 * last_idx) {
      ++num_digits;
      last_idx = net_idx;
    }

    std::size_t next_wire_sz = 2 /*, */ + net_prefix_sz + num_digits;
    if (wire_line.size() + next_wire_sz + 1 >= num_cols) {
      fmt::println(os, "{};", wire_line);
      wire_line = "  wire ";
    } else {
      wire_line.append(", ");
    }
    wire_line.append(net_prefix);
    wire_line.append(std::to_string(net_idx));
  }

  if (!wire_line.empty()) {
    fmt::println(os, "{};", wire_line);
  }
}

template <typename OSTREAM>
void write_cells(OSTREAM &os) {
  // the first cell is a special case, since it connects to the input port
  fmt::println(
      os,
      "  {} {}{}(.{}(A), .{}({}{}));",
      lib_cell_name,
      cell_prefix,
      1,
      lib_cell_inp_pin,
      lib_cell_out_pin,
      net_prefix,
      1);
  for (std::size_t net_idx = 2; net_idx < num_nets; ++net_idx) {
    fmt::println(
        os,
        "  {} {}{}(.{}({}{}), .{}({}{}));",
        lib_cell_name,
        cell_prefix,
        net_idx,
        lib_cell_inp_pin,
        net_prefix,
        net_idx / 2,
        lib_cell_out_pin,
        net_prefix,
        net_idx);
  }
  // leaf cells
  for (std::size_t net_idx = num_nets; net_idx < 2 * num_nets; ++net_idx) {
    fmt::println(
        os,
        "  {} {}{}(.{}({}{}), .{}());",
        lib_cell_name,
        cell_prefix,
        net_idx,
        lib_cell_inp_pin,
        net_prefix,
        net_idx / 2,
        lib_cell_out_pin);
  }
}

void write_block() {
#ifdef WRITE_COMPRESSED
  static constexpr std::string filename{block_name + ".v.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream os(&buf);
#else
  static constexpr std::string filename{block_name + ".v"};
  std::ofstream os(filename);
#endif
  fmt::println(os, "module {}(A);", block_name);
  fmt::println(os, "  input A;");
  write_wires(os);
  write_cells(os);
  fmt::println(os, "endmodule");
}

void write_top() {
#ifdef WRITE_COMPRESSED
  static constexpr std::string filename{top_name + ".v.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream os(&buf);
#else
  static constexpr std::string filename{top_name + ".v"};
  std::ofstream os(filename);
#endif
  fmt::println(os, "module {}(", top_name);
  for (std::size_t block_idx = 0; block_idx < num_blocks - 1; ++block_idx) {
    fmt::println(os, "  A{},", block_idx + 1);
  }
  fmt::println(os, "  A{});", num_blocks);

  std::string input_line;
  std::size_t num_digits = 1;
  std::size_t last_idx = 1;
  for (std::size_t block_idx = 1; block_idx <= num_blocks; ++block_idx) {
    if (input_line.empty()) {
      input_line = fmt::format("  input A{}", block_idx);
      continue;
    }

    if (block_idx >= 10 * last_idx) {
      ++num_digits;
      last_idx = block_idx;
    }

    std::size_t next_input_sz = 3 /*, A*/ + num_digits;
    if (input_line.size() + next_input_sz + 1 >= num_cols) {
      fmt::println(os, "{};", input_line);
      input_line = "  input ";
    } else {
      input_line.append(", ");
    }
    input_line += fmt::format("A{}", block_idx);
  }

  if (!input_line.empty()) {
    fmt::println(os, "{};", input_line);
  }

  for (std::size_t block_idx = 0; block_idx < num_blocks; ++block_idx) {
    fmt::println(
        os,
        "  {} {}{}(.A(A{}));",
        block_name,
        block_prefix,
        block_idx + 1,
        block_idx + 1);
  }
  fmt::println(os, "endmodule");
}

int main() {
  write_block();
  write_top();

  return 0;
}
