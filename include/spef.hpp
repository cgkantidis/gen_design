#ifndef SPEF_HPP
#define SPEF_HPP

#include <fmt/format.h>
#include <libassert/assert.hpp>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <utility>
#include <variant>
#include <vector>

class hier_div {
public:
  enum { DOT, SLASH, COLON, BAR } div;

  [[nodiscard]] char to_char() const {
    switch (div) {
    case DOT:
      return '.';
    case SLASH:
      return '/';
    case COLON:
      return ':';
    case BAR:
      return '|';
    }
    UNREACHABLE();
  }

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*DIVIDER {}", to_char());
  }
};

class pin_delim {
public:
  enum { DOT, SLASH, COLON, BAR } delim;

  [[nodiscard]] char to_char() const {
    switch (delim) {
    case DOT:
      return '.';
    case SLASH:
      return '/';
    case COLON:
      return ':';
    case BAR:
      return '|';
    }
    UNREACHABLE();
  }

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*DELIMITER {}", to_char());
  }
};

class bus_delim {
public:
  enum {
    SQUARE_BRACKET,
    CURLY_BRACKET,
    PARENTHESIS,
    ANGLE_BRACKET,
    COLON,
    DOT
  } delim;

  [[nodiscard]] std::string_view to_sv() const {
    switch (delim) {
    case SQUARE_BRACKET:
      return "[]";
    case CURLY_BRACKET:
      return "{}";
    case PARENTHESIS:
      return "()";
    case ANGLE_BRACKET:
      return "<>";
    case COLON:
      return ":";
    case DOT:
      return ".";
    }
    UNREACHABLE();
  }
  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*BUS_DELIMITER {}", to_sv());
  }
};

class time_scale {
public:
  double m_val;
  enum { NS, PS } m_time_unit;

  [[nodiscard]] std::string_view time_unit_sv() const {
    switch (m_time_unit) {
    case NS:
      return "NS";
    case PS:
      return "PS";
    }
    UNREACHABLE();
  }

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*T_UNIT {} {}", m_val, time_unit_sv());
  }
};

class cap_scale {
public:
  double m_val;
  enum { PF, FF } m_cap_unit;

  [[nodiscard]] std::string_view cap_unit_sv() const {
    switch (m_cap_unit) {
    case PF:
      return "PF";
    case FF:
      return "FF";
    }
    UNREACHABLE();
  }

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*C_UNIT {} {}", m_val, cap_unit_sv());
  }
};

class res_scale {
public:
  double m_val;
  enum { OHM, KOHM } m_res_unit;

  [[nodiscard]] std::string_view res_unit_sv() const {
    switch (m_res_unit) {
    case OHM:
      return "OHM";
    case KOHM:
      return "KOHM";
    }
    UNREACHABLE();
  }

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*R_UNIT {} {}", m_val, res_unit_sv());
  }
};

class induc_scale {
public:
  double m_val;
  enum { HENRY, MH, UH } m_induc_unit;

  [[nodiscard]] std::string_view induc_unit_sv() const {
    switch (m_induc_unit) {
    case HENRY:
      return "HENRY";
    case MH:
      return "MH";
    case UH:
      return "UH";
    }
    UNREACHABLE();
  }

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*L_UNIT {} {}", m_val, induc_unit_sv());
  }
};

class unit_def {
public:
  time_scale m_time_scale;
  cap_scale m_cap_scale;
  res_scale m_res_scale;
  induc_scale m_induc_scale;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    m_time_scale.write(os);
    m_cap_scale.write(os);
    m_res_scale.write(os);
    m_induc_scale.write(os);
  }
};

class design_flow {
public:
  std::vector<std::string> m_flow;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    if (m_flow.empty()) {
      fmt::println(os, R"(*DESIGN_FLOW "")");
    } else {
      fmt::print(os, "*DESIGN_FLOW");
      for (std::string const &flow : m_flow) {
        fmt::print(os, R"( "{}")", flow);
      }
      fmt::println(os, "");
    }
  }
};

class header_def {
public:
  std::string m_SPEF_version;
  std::string m_design_name;
  std::string m_date;
  std::string m_vendor;
  std::string m_program_name;
  std::string m_program_version;
  design_flow m_design_flow;
  hier_div m_hier_div;
  pin_delim m_pin_delim;
  bus_delim m_bus_delim;
  unit_def m_unit_def;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, R"(*SPEF "{}")", m_SPEF_version);
    fmt::println(os, R"(*DESIGN "{}")", m_design_name);
    fmt::println(os, R"(*DATE "{}")", m_date);
    fmt::println(os, R"(*VENDOR "{}")", m_vendor);
    fmt::println(os, R"(*PROGRAM "{}")", m_program_name);
    fmt::println(os, R"(*VERSION "{}")", m_program_version);
    m_design_flow.write(os);
    m_hier_div.write(os);
    m_pin_delim.write(os);
    m_bus_delim.write(os);
    m_unit_def.write(os);
  }
};

class name_map {
public:
  std::map<std::uint64_t, std::string> m_map;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    if (m_map.empty()) {
      return;
    }
    fmt::println(os, "*NAME_MAP");
    for (auto const &[index, name] : m_map) {
      fmt::println(os, "*{} {}", index, name);
    }
  }
};

class power_def {
public:
  std::vector<std::string> m_power_nets;
  std::vector<std::string> m_ground_nets;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    if (!m_power_nets.empty()) {
      fmt::print(os, "*POWER_NETS");
      for (std::string const &power_net : m_power_nets) {
        fmt::print(os, " {}", power_net);
      }
      fmt::println(os, "");
    }
    if (!m_ground_nets.empty()) {
      fmt::print(os, "*GROUND_NETS");
      for (std::string const &ground_net : m_ground_nets) {
        fmt::print(os, " {}", ground_net);
      }
      fmt::println(os, "");
    }
  }
};

class direction {
public:
  enum { I, B, O } dir;

  [[nodiscard]] char to_char() const {
    switch (dir) {
    case I:
      return 'I';
    case B:
      return 'B';
    case O:
      return 'O';
    }
    UNREACHABLE();
  }
};

class coordinates {
public:
  std::uint64_t x;
  std::uint64_t y;

  [[nodiscard]] std::string to_string() const {
    return fmt::format("*C {} {}", x, y);
  }
};

template <typename T>
class multivalue {
public:
  // TODO: we should use a data structure that has constant size
  std::vector<T> m_value;
  explicit multivalue(std::size_t size, T val = {}) : m_value(size, val) {}
  multivalue(std::initializer_list<T> value) : m_value(value) {}

  [[nodiscard]] std::string to_string() const {
    ASSERT(!m_value.empty());

    std::string ret_str = fmt::format("{:.1f}", m_value[0]);
    for (auto const &val : m_value | std::views::drop(1)) {
      ret_str += fmt::format(":{:.1f}", val);
    }
    return ret_str;
  }

  [[nodiscard]] multivalue<T> operator+(multivalue<T> const &other) const {
    ASSERT(m_value.size() == other.m_value.size());
    multivalue<T> ret_value(m_value.size());
    ret_value.m_value.resize(m_value.size());
    for (std::size_t idx = 0; idx < ret_value.m_value.size(); ++idx) {
      ret_value.m_value[idx] = m_value[idx] + other.m_value[idx];
    }
    return ret_value;
  }
};

using par_value = multivalue<double>;

class cap_load {
public:
  par_value m_par_value;

  [[nodiscard]] std::string to_string() const {
    return fmt::format("*L {}", m_par_value.to_string());
  }
};

using pos_fraction = multivalue<double>;

class threshold {
public:
  pos_fraction m_value;

  [[nodiscard]] std::string to_string() const {
    return m_value.to_string();
  }
};

class slews {
public:
  std::pair<par_value, par_value> m_slews;
  std::optional<std::pair<threshold, threshold>> m_thresholds;

  [[nodiscard]] std::string to_string() const {
    std::string ret_str;
    ret_str += fmt::format(
        "*S {} {}",
        m_slews.first.to_string(),
        m_slews.second.to_string());
    if (m_thresholds) {
      ret_str += fmt::format(
          " {} {}",
          m_thresholds->first.to_string(),
          m_thresholds->second.to_string());
    }
    return ret_str;
  }
};

class driving_cell {
public:
  std::string m_cell_type;

  [[nodiscard]] std::string to_string() const {
    return fmt::format("*D {}", m_cell_type);
  }
};

class conn_attr {
public:
  std::variant<coordinates, cap_load, slews, driving_cell> m_attr;

  [[nodiscard]] std::string to_string() const {
    return std::visit(
        [](auto &&arg) -> std::string { return arg.to_string(); },
        m_attr);
  }
};

class port_entry {
public:
  std::string m_port_name;
  direction m_direction;
  std::optional<conn_attr> m_conn_attr;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    if (m_conn_attr) {
      fmt::println(
          os,
          "{} {} {}",
          m_port_name,
          m_direction.to_char(),
          m_conn_attr->to_string());
    } else {
      fmt::println(os, "{} {}", m_port_name, m_direction.to_char());
    }
  }
};

class port_def {
public:
  std::vector<port_entry> m_port_entries;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    if (m_port_entries.empty()) {
      return;
    }

    fmt::println(os, "*PORTS");
    for (port_entry const &entry : m_port_entries) {
      entry.write(os);
    }
  }
};

class external_def {
public:
  port_def m_port_def;
  // physical_port_def m_port_def;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    m_port_def.write(os);
  }
};

class conn_def {
public:
  bool m_is_external;
  std::string m_name;
  direction m_direction;
  std::optional<conn_attr> m_conn_attr;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::print(
        os,
        "*{} {} {}",
        m_is_external ? 'P' : 'I',
        m_name,
        m_direction.to_char());
    if (m_conn_attr) {
      fmt::print(os, " {}", m_conn_attr->to_string());
    }
    fmt::println(os, "");
  }
};

class internal_node_coord {
public:
  std::pair<std::string, coordinates> m_internal_node;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(
        os,
        "*N {} {}",
        m_internal_node.first,
        m_internal_node.second.to_string());
  }
};

class conn_sec {
public:
  std::vector<conn_def> m_conn_def;
  std::vector<internal_node_coord> m_internal_node_coord;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*CONN");
    for (conn_def const &def : m_conn_def) {
      def.write(os);
    }
    for (internal_node_coord const &internal_node : m_internal_node_coord) {
      internal_node.write(os);
    }
  }
};

class cap {
public:
  std::string m_node1;
  std::optional<std::string> m_node2;
  par_value m_par_value;

  cap(std::string node1, std::initializer_list<double> value)
      : m_node1(std::move(node1)),
        m_par_value(value) {}
  cap(std::string node1, std::string node2, std::initializer_list<double> value)
      : m_node1(std::move(node1)),
        m_node2(std::move(node2)),
        m_par_value(value) {}

  [[nodiscard]] std::string to_string() const {
    if (m_node2) {
      return fmt::format(
          "{} {} {}",
          m_node1,
          *m_node2,
          m_par_value.to_string());
    }
    return fmt::format("{} {}", m_node1, m_par_value.to_string());
  }
};

class cap_sec {
public:
  std::vector<cap> m_caps;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*CAP");
    std::size_t idx = 1;
    for (cap const &c : m_caps) {
      fmt::println(os, "{} {}", idx++, c.to_string());
    }
  }
};

class res {
public:
  std::string m_node1;
  std::string m_node2;
  par_value m_par_value;

  res(std::string node1, std::string node2, std::initializer_list<double> value)
      : m_node1(std::move(node1)),
        m_node2(std::move(node2)),
        m_par_value(value) {}

  [[nodiscard]] std::string to_string() const {
    return fmt::format("{} {} {}", m_node1, m_node2, m_par_value.to_string());
  }
};

class res_sec {
public:
  std::vector<res> m_ress;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*RES");
    std::size_t idx = 1;
    for (res const &r : m_ress) {
      fmt::println(os, "{} {}", idx++, r.to_string());
    }
  }
};

class d_net {
public:
  std::string m_net_ref;
  // routing_conf m_routing_conf;
  conn_sec m_conn_sec;
  cap_sec m_cap_sec;
  res_sec m_res_sec;
  // induc_sec m_induc_sec;

  [[nodiscard]] par_value total_cap() const {
    std::size_t num_corners = m_cap_sec.m_caps[0].m_par_value.m_value.size();
    par_value total(num_corners);

    for (cap const &c : m_cap_sec.m_caps) {
      total = total + c.m_par_value;
    }
    return total;
  }

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    fmt::println(os, "*D_NET {} {}", m_net_ref, total_cap().to_string());
    m_conn_sec.write(os);
    m_cap_sec.write(os);
    m_res_sec.write(os);
    fmt::println(os, "*END");
  }
};

class internal_def {
public:
  std::vector<d_net> m_d_nets;
  // std::vector<r_net> m_r_nets;
  // std::vector<d_pnet> m_d_pnets;
  // std::vector<r_pnet> m_r_pnets;
  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    for (d_net const &net : m_d_nets) {
      net.write(os);
    }
  }
};

class SPEF_file {
public:
  header_def m_header_def;
  name_map m_name_map;
  power_def m_power_def;
  external_def m_external_def;
  // std::optional<define_def> m_define_def;
  // std::optional<variation_def> m_variation_def;
  internal_def m_internal_def;

  template <typename OSTREAM>
  void write(OSTREAM &os) const {
    m_header_def.write(os);
    m_name_map.write(os);
    m_power_def.write(os);
    m_external_def.write(os);
    m_internal_def.write(os);
  }
};
#endif  // SPEF_HPP
