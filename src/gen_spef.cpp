#include <fmt/ostream.h>
#include <random>
#include <string>

#include "common.hpp"
#include "spef.hpp"

// generate random value
double rv() {
  static std::random_device rd;
  static std::mt19937_64 gen(rd());
  static std::uniform_real_distribution<> dis(1.0, 5.0);
  return dis(gen);
}

void gen_header(SPEF_file &spef, std::string design_name) {
  spef.m_header_def.m_SPEF_version = "IEEE 1481-1999";
  spef.m_header_def.m_design_name = std::move(design_name);
  {
    std::time_t now = std::time(nullptr);
    std::array<char, 80> date_buf;
    if (std::strftime(
            date_buf.data(),
            date_buf.size(),
            "%a %b %d %H:%M:%S %Y",
            std::localtime(&now))
        > 0) {
      spef.m_header_def.m_date = date_buf.data();
    }
  }
  spef.m_header_def.m_vendor = "Synopsys Inc.";
  spef.m_header_def.m_program_name = "gen_spef";
  spef.m_header_def.m_program_version = "1.0.0";
  spef.m_header_def.m_hier_div = hier_div{hier_div::SLASH};
  spef.m_header_def.m_pin_delim = pin_delim{pin_delim::COLON};
  spef.m_header_def.m_bus_delim = bus_delim{bus_delim::SQUARE_BRACKET};
  spef.m_header_def.m_unit_def.m_time_scale = {1.0, time_scale::NS};
  spef.m_header_def.m_unit_def.m_cap_scale = {1.0, cap_scale::PF};
  spef.m_header_def.m_unit_def.m_res_scale = {1.0, res_scale::KOHM};
  spef.m_header_def.m_unit_def.m_induc_scale = {1.0, induc_scale::HENRY};
}

void gen_block_ports(SPEF_file &spef) {
  spef.m_external_def.m_port_def.m_port_entries.emplace_back(
      port_entry("A", direction{direction::O}, {}));
}

void gen_top_ports(SPEF_file &spef) {
  for (std::size_t block_idx = 0; block_idx < num_blocks; ++block_idx) {
    spef.m_external_def.m_port_def.m_port_entries.emplace_back(port_entry(
        fmt::format("A{}", block_idx + 1),
        direction{direction::O},
        {}));
  }
}

void gen_block_net_net_ref(d_net &net, std::size_t net_idx) {
  if (net_idx != 0) {
    net.m_net_ref = fmt::format("{}{}", net_prefix, net_idx);
  } else {
    net.m_net_ref = "A";
  }
}

void gen_top_net_net_ref(d_net &net, std::size_t block_idx) {
  net.m_net_ref = fmt::format("A{}", block_idx + 1);
}

void gen_block_net_conn_def(
    d_net &net,
    std::size_t net_idx,
    char pin_delim_ch) {
  if (net_idx == 0) {
    std::string driver_pin = "A";
    std::string load_pin =
        fmt::format("{}1{}{}", cell_prefix, pin_delim_ch, lib_cell_inp_pin);
    net.m_conn_sec.m_conn_def.emplace_back(
        conn_def(true, driver_pin, {direction::O}, {}));
    net.m_conn_sec.m_conn_def.emplace_back(
        conn_def(false, load_pin, {direction::I}, {}));
    return;
  }

  bool const is_net_to_leaf_cell = net_idx >= num_nets / 2;

  std::string driver_pin = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx,
      pin_delim_ch,
      lib_cell_out_pin);
  std::string load_pin1 = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx * 2,
      pin_delim_ch,
      is_net_to_leaf_cell ? lib_leaf_cell_d_pin : lib_cell_inp_pin);
  std::string load_pin2 = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx * 2 + 1,
      pin_delim_ch,
      is_net_to_leaf_cell ? lib_leaf_cell_d_pin : lib_cell_inp_pin);
  std::string internal_node =
      fmt::format("{}{}{}1", net_prefix, net_idx, pin_delim_ch);

  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(false, driver_pin, {direction::O}, {}));
  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(false, load_pin1, {direction::I}, {}));
  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(false, load_pin2, {direction::I}, {}));
  net.m_conn_sec.m_internal_node_coord.emplace_back(
      internal_node_coord({internal_node, {0, 0}}));
}

void gen_top_net_conn_def(d_net &net, std::size_t block_idx, char hier_div_ch) {
  std::string driver_pin = fmt::format("A{}", block_idx + 1);
  std::string load_pin =
      fmt::format("{}{}{}A", block_prefix, block_idx + 1, hier_div_ch);
  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(true, driver_pin, {direction::O}, {}));
  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(false, load_pin, {direction::I}, {}));
}

void gen_block_net_cap_sec_ground(
    d_net &net,
    std::size_t net_idx,
    char pin_delim_ch) {
  if (net_idx == 0) {
    std::string driver_pin = "A";
    std::string load_pin =
        fmt::format("{}1{}{}", cell_prefix, pin_delim_ch, lib_cell_inp_pin);
    net.m_cap_sec.m_caps.emplace_back(cap(driver_pin, {rv()}));
    net.m_cap_sec.m_caps.emplace_back(cap(load_pin, {rv()}));
    return;
  }

  bool const is_net_to_leaf_cell = net_idx >= num_nets / 2;

  std::string driver_pin = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx,
      pin_delim_ch,
      lib_cell_out_pin);
  std::string load_pin1 = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx * 2,
      pin_delim_ch,
      is_net_to_leaf_cell ? lib_leaf_cell_d_pin : lib_cell_inp_pin);
  std::string load_pin2 = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx * 2 + 1,
      pin_delim_ch,
      is_net_to_leaf_cell ? lib_leaf_cell_d_pin : lib_cell_inp_pin);
  std::string internal_node =
      fmt::format("{}{}{}1", net_prefix, net_idx, pin_delim_ch);

  net.m_cap_sec.m_caps.emplace_back(cap(driver_pin, {rv()}));
  net.m_cap_sec.m_caps.emplace_back(cap(load_pin1, {rv()}));
  net.m_cap_sec.m_caps.emplace_back(cap(load_pin2, {rv()}));
  net.m_cap_sec.m_caps.emplace_back(cap(internal_node, {rv()}));
}

void gen_top_net_cap_sec_ground(
    d_net &net,
    std::size_t block_idx,
    char hier_div_ch) {
  std::string driver_pin = fmt::format("A{}", block_idx + 1);
  std::string load_pin =
      fmt::format("{}{}{}A", block_prefix, block_idx + 1, hier_div_ch);
  net.m_cap_sec.m_caps.emplace_back(cap(driver_pin, {rv()}));
  net.m_cap_sec.m_caps.emplace_back(cap(load_pin, {rv()}));
}

void gen_block_net_res_sec(d_net &net, std::size_t net_idx, char pin_delim_ch) {
  if (net_idx == 0) {
    std::string driver_pin = "A";
    std::string load_pin =
        fmt::format("{}1{}{}", cell_prefix, pin_delim_ch, lib_cell_inp_pin);
    net.m_res_sec.m_ress.emplace_back(res(driver_pin, load_pin, {rv()}));
    return;
  }

  bool const is_net_to_leaf_cell = net_idx >= num_nets / 2;

  std::string driver_pin = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx,
      pin_delim_ch,
      lib_cell_out_pin);
  std::string load_pin1 = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx * 2,
      pin_delim_ch,
      is_net_to_leaf_cell ? lib_leaf_cell_d_pin : lib_cell_inp_pin);
  std::string load_pin2 = fmt::format(
      "{}{}{}{}",
      cell_prefix,
      net_idx * 2 + 1,
      pin_delim_ch,
      is_net_to_leaf_cell ? lib_leaf_cell_d_pin : lib_cell_inp_pin);
  std::string internal_node =
      fmt::format("{}{}{}1", net_prefix, net_idx, pin_delim_ch);

  net.m_res_sec.m_ress.emplace_back(res(driver_pin, internal_node, {rv()}));
  net.m_res_sec.m_ress.emplace_back(res(internal_node, load_pin1, {rv()}));
  net.m_res_sec.m_ress.emplace_back(res(internal_node, load_pin2, {rv()}));
}

void gen_top_net_res_sec(d_net &net, std::size_t block_idx, char hier_div_ch) {
  std::string driver_pin = fmt::format("A{}", block_idx + 1);
  std::string load_pin =
      fmt::format("{}{}{}A", block_prefix, block_idx + 1, hier_div_ch);
  net.m_res_sec.m_ress.emplace_back(res(driver_pin, load_pin, {rv()}));
}

void gen_block_nets(SPEF_file &spef) {
  spef.m_internal_def.m_d_nets.resize(num_nets);
  char pin_delim_ch = spef.m_header_def.m_pin_delim.to_char();
  for (std::size_t net_idx = 0; net_idx < num_nets; ++net_idx) {
    d_net &net = spef.m_internal_def.m_d_nets[net_idx];
    gen_block_net_net_ref(net, net_idx);
    gen_block_net_conn_def(net, net_idx, pin_delim_ch);
    gen_block_net_cap_sec_ground(net, net_idx, pin_delim_ch);
    gen_block_net_res_sec(net, net_idx, pin_delim_ch);
  }
}

void gen_top_nets(SPEF_file &spef) {
  spef.m_internal_def.m_d_nets.resize(num_blocks);
  char hier_div_ch = spef.m_header_def.m_hier_div.to_char();
  for (std::size_t block_idx = 0; block_idx < num_blocks; ++block_idx) {
    d_net &net = spef.m_internal_def.m_d_nets[block_idx];
    gen_top_net_net_ref(net, block_idx);
    gen_top_net_conn_def(net, block_idx, hier_div_ch);
    gen_top_net_cap_sec_ground(net, block_idx, hier_div_ch);
    gen_top_net_res_sec(net, block_idx, hier_div_ch);
  }
}

void write_block_spef() {
#ifdef WRITE_COMPRESSED
  static constexpr std::string filename{block_name + ".spef.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream os(&buf);
#else
  static constexpr std::string filename{block_name + ".spef"};
  std::ofstream os(filename);
#endif

  SPEF_file spef;
  gen_header(spef, block_name);
  gen_block_ports(spef);
  gen_block_nets(spef);
  spef.write(os);
}

void write_top_spef() {
#ifdef WRITE_COMPRESSED
  static constexpr std::string filename{top_name + ".spef.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream os(&buf);
#else
  static constexpr std::string filename{top_name + ".spef"};
  std::ofstream os(filename);
#endif

  SPEF_file spef;
  gen_header(spef, top_name);
  gen_top_ports(spef);
  gen_top_nets(spef);
  spef.write(os);
}

int main() {
  write_block_spef();
  write_top_spef();
  return 0;
}
