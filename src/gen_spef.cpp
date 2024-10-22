#ifdef WRITE_COMPRESSED
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <ostream>
#else
#include <fstream>
#endif

#include <fmt/ostream.h>
#include <random>
#include <string>

#include "design_config.hpp"
#include "spef.hpp"

// forward declarations
void gen_header(SPEF_file &spef, std::string design_name);
void gen_block_ports(SPEF_file &spef);
void gen_top_ports(SPEF_file &spef, design_config const &config);
void gen_block_nets(SPEF_file &spef, design_config const &config);
void gen_top_nets(SPEF_file &spef, design_config const &config);
void gen_block_net_net_ref(
    d_net &net,
    std::size_t net_idx,
    design_config const &config);
void gen_top_net_net_ref(d_net &net, std::size_t block_idx);
void gen_block_net_conn_def(
    d_net &net,
    std::size_t net_idx,
    char pin_delim_ch,
    design_config const &config);
void gen_top_net_conn_def(
    d_net &net,
    std::size_t block_idx,
    char hier_div_ch,
    design_config const &config);
void gen_block_net_cap_sec_ground(
    d_net &net,
    std::size_t net_idx,
    char pin_delim_ch,
    design_config const &config);
void gen_top_net_cap_sec_ground(
    d_net &net,
    std::size_t block_idx,
    char hier_div_ch,
    design_config const &config);
void gen_block_net_res_sec(
    d_net &net,
    std::size_t net_idx,
    char pin_delim_ch,
    design_config const &config);
void gen_top_net_res_sec(
    d_net &net,
    std::size_t block_idx,
    char hier_div_ch,
    design_config const &config);
void gen_block_net_cap_sec_coupling(
    std::vector<d_net> &nets,
    design_config const &config);
void gen_top_net_cap_sec_coupling(
    std::vector<d_net> &nets,
    design_config const &config);

// RNG helpers
double rand_cap();
std::mt19937_64 &get_gen();
std::uniform_int_distribution<std::size_t>
get_idx_dist(std::size_t min_idx, std::size_t max_idx);
std::uniform_real_distribution<double> &get_cap_dist();
std::string const &get_rand_node(conn_sec const &conns);

void write_block_spef(design_config const &config) {
#ifdef WRITE_COMPRESSED
  std::string filename{config.block_name + ".spef.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream os(&buf);
#else
  std::string filename{config.block_name + ".spef"};
  std::ofstream os(filename);
#endif

  SPEF_file spef;
  gen_header(spef, config.block_name);
  gen_block_ports(spef);
  gen_block_nets(spef, config);
  spef.write(os);
}

void write_top_spef(design_config const &config) {
#ifdef WRITE_COMPRESSED
  std::string filename{config.top_name + ".spef.gz"};
  boost::iostreams::filtering_ostreambuf buf;
  buf.push(boost::iostreams::gzip_compressor());
  buf.push(boost::iostreams::file_sink(filename));
  std::ostream os(&buf);
#else
  std::string filename{config.top_name + ".spef"};
  std::ofstream os(filename);
#endif

  SPEF_file spef;
  gen_header(spef, config.top_name);
  gen_top_ports(spef, config);
  gen_top_nets(spef, config);
  spef.write(os);
}

void gen_header(SPEF_file &spef, std::string design_name) {
  spef.m_header_def.m_SPEF_version = "IEEE 1481-1999";
  spef.m_header_def.m_design_name = std::move(design_name);
  {
    std::time_t now = std::time(nullptr);
    std::array<char, 80> date_buf{};
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

void gen_top_ports(SPEF_file &spef, design_config const &config) {
  for (std::size_t block_idx = 0; block_idx < config.num_blocks; ++block_idx) {
    spef.m_external_def.m_port_def.m_port_entries.emplace_back(port_entry(
        fmt::format("A{}", block_idx + 1),
        direction{direction::O},
        {}));
  }
}

void gen_block_nets(SPEF_file &spef, design_config const &config) {
  spef.m_internal_def.m_d_nets.resize(config.num_nets);
  char pin_delim_ch = spef.m_header_def.m_pin_delim.to_char();
  for (std::size_t net_idx = 0; net_idx < config.num_nets; ++net_idx) {
    d_net &net = spef.m_internal_def.m_d_nets[net_idx];
    gen_block_net_net_ref(net, net_idx, config);
    gen_block_net_conn_def(net, net_idx, pin_delim_ch, config);
    gen_block_net_cap_sec_ground(net, net_idx, pin_delim_ch, config);
    gen_block_net_res_sec(net, net_idx, pin_delim_ch, config);
  }
  gen_block_net_cap_sec_coupling(spef.m_internal_def.m_d_nets, config);
}

void gen_top_nets(SPEF_file &spef, design_config const &config) {
  spef.m_internal_def.m_d_nets.resize(config.num_blocks);
  char hier_div_ch = spef.m_header_def.m_hier_div.to_char();
  for (std::size_t block_idx = 0; block_idx < config.num_blocks; ++block_idx) {
    d_net &net = spef.m_internal_def.m_d_nets[block_idx];
    gen_top_net_net_ref(net, block_idx);
    gen_top_net_conn_def(net, block_idx, hier_div_ch, config);
    gen_top_net_cap_sec_ground(net, block_idx, hier_div_ch, config);
    gen_top_net_res_sec(net, block_idx, hier_div_ch, config);
  }
  gen_top_net_cap_sec_coupling(spef.m_internal_def.m_d_nets, config);
}

void gen_block_net_net_ref(
    d_net &net,
    std::size_t net_idx,
    design_config const &config) {
  if (net_idx != 0) {
    net.m_net_ref = fmt::format("{}{}", config.net_prefix, net_idx);
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
    char pin_delim_ch,
    design_config const &config) {
  if (net_idx == 0) {
    std::string driver_pin = "A";
    std::string load_pin = fmt::format(
        "{}1{}{}",
        config.cell_prefix,
        pin_delim_ch,
        config.lib_cell_inp_pin);
    net.m_conn_sec.m_conn_def.emplace_back(
        conn_def(true, driver_pin, {direction::O}, {}));
    net.m_conn_sec.m_conn_def.emplace_back(
        conn_def(false, load_pin, {direction::I}, {}));
    return;
  }

  bool const is_net_to_leaf_cell = net_idx >= config.num_nets / 2;

  std::string driver_pin = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx,
      pin_delim_ch,
      config.lib_cell_out_pin);
  std::string load_pin1 = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx * 2,
      pin_delim_ch,
      is_net_to_leaf_cell ? config.lib_leaf_cell_d_pin
                          : config.lib_cell_inp_pin);
  std::string load_pin2 = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx * 2 + 1,
      pin_delim_ch,
      is_net_to_leaf_cell ? config.lib_leaf_cell_d_pin
                          : config.lib_cell_inp_pin);
  std::string internal_node =
      fmt::format("{}{}{}1", config.net_prefix, net_idx, pin_delim_ch);

  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(false, driver_pin, {direction::O}, {}));
  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(false, load_pin1, {direction::I}, {}));
  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(false, load_pin2, {direction::I}, {}));
  net.m_conn_sec.m_internal_node_coord.emplace_back(
      internal_node_coord({internal_node, {0, 0}}));
}

void gen_top_net_conn_def(
    d_net &net,
    std::size_t block_idx,
    char hier_div_ch,
    design_config const &config) {
  std::string driver_pin = fmt::format("A{}", block_idx + 1);
  std::string load_pin =
      fmt::format("{}{}{}A", config.block_prefix, block_idx + 1, hier_div_ch);
  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(true, driver_pin, {direction::O}, {}));
  net.m_conn_sec.m_conn_def.emplace_back(
      conn_def(false, load_pin, {direction::I}, {}));
}

void gen_block_net_cap_sec_ground(
    d_net &net,
    std::size_t net_idx,
    char pin_delim_ch,
    design_config const &config) {
  if (net_idx == 0) {
    std::string driver_pin = "A";
    std::string load_pin = fmt::format(
        "{}1{}{}",
        config.cell_prefix,
        pin_delim_ch,
        config.lib_cell_inp_pin);
    net.m_cap_sec.m_caps.emplace_back(cap(driver_pin, {rand_cap()}));
    net.m_cap_sec.m_caps.emplace_back(cap(load_pin, {rand_cap()}));
    return;
  }

  bool const is_net_to_leaf_cell = net_idx >= config.num_nets / 2;

  std::string driver_pin = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx,
      pin_delim_ch,
      config.lib_cell_out_pin);
  std::string load_pin1 = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx * 2,
      pin_delim_ch,
      is_net_to_leaf_cell ? config.lib_leaf_cell_d_pin
                          : config.lib_cell_inp_pin);
  std::string load_pin2 = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx * 2 + 1,
      pin_delim_ch,
      is_net_to_leaf_cell ? config.lib_leaf_cell_d_pin
                          : config.lib_cell_inp_pin);
  std::string internal_node =
      fmt::format("{}{}{}1", config.net_prefix, net_idx, pin_delim_ch);

  net.m_cap_sec.m_caps.emplace_back(cap(driver_pin, {rand_cap()}));
  net.m_cap_sec.m_caps.emplace_back(cap(load_pin1, {rand_cap()}));
  net.m_cap_sec.m_caps.emplace_back(cap(load_pin2, {rand_cap()}));
  net.m_cap_sec.m_caps.emplace_back(cap(internal_node, {rand_cap()}));
}

void gen_top_net_cap_sec_ground(
    d_net &net,
    std::size_t block_idx,
    char hier_div_ch,
    design_config const &config) {
  std::string driver_pin = fmt::format("A{}", block_idx + 1);
  std::string load_pin =
      fmt::format("{}{}{}A", config.block_prefix, block_idx + 1, hier_div_ch);
  net.m_cap_sec.m_caps.emplace_back(cap(driver_pin, {rand_cap()}));
  net.m_cap_sec.m_caps.emplace_back(cap(load_pin, {rand_cap()}));
}

void gen_block_net_res_sec(
    d_net &net,
    std::size_t net_idx,
    char pin_delim_ch,
    design_config const &config) {
  if (net_idx == 0) {
    std::string driver_pin = "A";
    std::string load_pin = fmt::format(
        "{}1{}{}",
        config.cell_prefix,
        pin_delim_ch,
        config.lib_cell_inp_pin);
    net.m_res_sec.m_ress.emplace_back(res(driver_pin, load_pin, {rand_cap()}));
    return;
  }

  bool const is_net_to_leaf_cell = net_idx >= config.num_nets / 2;

  std::string driver_pin = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx,
      pin_delim_ch,
      config.lib_cell_out_pin);
  std::string load_pin1 = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx * 2,
      pin_delim_ch,
      is_net_to_leaf_cell ? config.lib_leaf_cell_d_pin
                          : config.lib_cell_inp_pin);
  std::string load_pin2 = fmt::format(
      "{}{}{}{}",
      config.cell_prefix,
      net_idx * 2 + 1,
      pin_delim_ch,
      is_net_to_leaf_cell ? config.lib_leaf_cell_d_pin
                          : config.lib_cell_inp_pin);
  std::string internal_node =
      fmt::format("{}{}{}1", config.net_prefix, net_idx, pin_delim_ch);

  net.m_res_sec.m_ress.emplace_back(
      res(driver_pin, internal_node, {rand_cap()}));
  net.m_res_sec.m_ress.emplace_back(
      res(internal_node, load_pin1, {rand_cap()}));
  net.m_res_sec.m_ress.emplace_back(
      res(internal_node, load_pin2, {rand_cap()}));
}

void gen_top_net_res_sec(
    d_net &net,
    std::size_t block_idx,
    char hier_div_ch,
    design_config const &config) {
  std::string driver_pin = fmt::format("A{}", block_idx + 1);
  std::string load_pin =
      fmt::format("{}{}{}A", config.block_prefix, block_idx + 1, hier_div_ch);
  net.m_res_sec.m_ress.emplace_back(res(driver_pin, load_pin, {rand_cap()}));
}

void gen_block_net_cap_sec_coupling(
    std::vector<d_net> &nets,
    design_config const &config) {
  auto &gen = get_gen();
  auto net_idx_dist = get_idx_dist(0, nets.size() - 1);

  for (std::size_t idx1 = 0; idx1 < nets.size(); ++idx1) {
    // we have 4 ground caps already
    while (nets[idx1].m_cap_sec.m_caps.size() < config.min_num_ccaps + 4) {

      std::size_t idx2 = net_idx_dist(gen);
      // don't generate self-coupling caps
      while (idx1 == idx2) {
        idx2 = net_idx_dist(gen);
      }

      d_net &net1 = nets[idx1];
      d_net &net2 = nets[idx2];

      std::string const &node1 = get_rand_node(net1.m_conn_sec);
      std::string const &node2 = get_rand_node(net2.m_conn_sec);
      auto caps = {rand_cap()};
      net1.m_cap_sec.m_caps.emplace_back(node1, node2, caps);
      net2.m_cap_sec.m_caps.emplace_back(node2, node1, caps);
    }
  }
}

void gen_top_net_cap_sec_coupling(
    std::vector<d_net> &nets,
    design_config const &config) {
  auto &gen = get_gen();
  auto net_idx_dist = get_idx_dist(0, nets.size() - 1);

  for (std::size_t idx1 = 0; idx1 < nets.size(); ++idx1) {
    // we have 2 ground caps already
    while (nets[idx1].m_cap_sec.m_caps.size() < config.min_num_ccaps + 2) {
      std::size_t idx2 = net_idx_dist(gen);
      // don't generate self-coupling caps
      while (idx1 == idx2) {
        idx2 = net_idx_dist(gen);
      }

      d_net &net1 = nets[idx1];
      d_net &net2 = nets[idx2];

      std::string const &node1 = get_rand_node(net1.m_conn_sec);
      std::string const &node2 = get_rand_node(net2.m_conn_sec);
      auto caps = {rand_cap()};
      net1.m_cap_sec.m_caps.emplace_back(node1, node2, caps);
      net2.m_cap_sec.m_caps.emplace_back(node2, node1, caps);
    }
  }
}

double rand_cap() {
  return get_cap_dist()(get_gen());
}

std::mt19937_64 &get_gen() {
  // TODO: add command line option to specify seed value for the RNG to
  // reproduce the same SPEF
  static std::random_device rd;
  static std::mt19937_64 gen(rd());
  return gen;
}

std::uniform_int_distribution<std::size_t>
get_idx_dist(std::size_t min_idx, std::size_t max_idx) {
  return std::uniform_int_distribution(min_idx, max_idx);
}

std::uniform_real_distribution<double> &get_cap_dist() {
  static std::uniform_real_distribution<> dist(MIN_CAP_VAL, MAX_CAP_VAL);
  return dist;
}

std::string const &get_rand_node(conn_sec const &conns) {
  auto const &pins = conns.m_conn_def;
  auto const &nodes = conns.m_internal_node_coord;
  auto const num_pins = pins.size();
  auto const num_nodes = num_pins + nodes.size();

  auto &gen = get_gen();
  auto node_idx_dist = get_idx_dist(0, num_nodes - 1);
  auto node_idx = node_idx_dist(gen);
  if (node_idx < num_pins) {
    return pins[node_idx].m_name;
  }
  return nodes[node_idx - num_pins].m_internal_node.first;
}
