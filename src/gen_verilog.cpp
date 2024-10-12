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

static constexpr std::size_t num_nets{100'000'000};
static constexpr std::size_t num_cols{80};
static constexpr std::string module_name{"top"};
static constexpr std::string cell_prefix{"u"};
static constexpr std::string net_prefix{"n"};
static constexpr std::string lib_cell_name{"IV"};
static constexpr std::string lib_cell_inp_pin{"A"};
static constexpr std::string lib_cell_out_pin{"Z"};

template <typename OSTREAM>
void write_wires(OSTREAM &outfile) {
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
      fmt::println(outfile, "{};", wire_line);
      wire_line = "  wire ";
    } else {
      wire_line.append(", ");
    }
    wire_line.append(net_prefix);
    wire_line.append(std::to_string(net_idx));
  }

  if (!wire_line.empty()) {
    fmt::println(outfile, "{};", wire_line);
  }
}

template <typename OSTREAM>
void write_cells(OSTREAM &outfile) {
  // the first cell is a special case, since it connects to the input port
  fmt::println(
      outfile,
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
        outfile,
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
        outfile,
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

int main() {
#ifdef WRITE_COMPRESSED
  static constexpr std::string filename{module_name + ".v.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream outfile(&buf);
#else
  static constexpr std::string filename{module_name + ".v"};
  std::ofstream outfile(filename);
#endif

  fmt::println(outfile, "module {}(A);", module_name);
  fmt::println(outfile, "  input A;");
  write_wires(outfile);
  write_cells(outfile);
  fmt::println(outfile, "endmodule");

  return 0;
}
