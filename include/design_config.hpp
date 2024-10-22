#ifndef DESIGN_CONFIG_HPP
#define DESIGN_CONFIG_HPP

#include <string>

// default config values

// by default the design will have `num_blocks * num_nets` nets by the end, and
// `num_blocks * num_nets + num_blocks` net segments, since
// 1'000'000 x 4'500 = 4.5B, which is a little larger than 2^32
static constexpr std::size_t NUM_NETS{1'000'000};
static constexpr std::size_t NUM_BLOCKS{4'500};
static constexpr std::size_t NUM_COLS{80};
static constexpr std::string BLOCK_NAME{"block"};
static constexpr std::string TOP_NAME{"top"};
static constexpr std::string BLOCK_PREFIX{"b"};
static constexpr std::string CELL_PREFIX{"u"};
static constexpr std::string NET_PREFIX{"n"};
static constexpr std::string LIB_CELL_NAME{"IV"};
static constexpr std::string LIB_CELL_INP_PIN{"A"};
static constexpr std::string LIB_CELL_OUT_PIN{"Z"};
static constexpr std::string LIB_LEAF_CELL_NAME{"FD1"};
static constexpr std::string LIB_LEAF_CELL_D_PIN{"D"};
static constexpr double MIN_CAP_VAL{1.0};
static constexpr double MAX_CAP_VAL{5.0};
static constexpr std::size_t MIN_NUM_CCAPS{5};

class design_config {
public:
  std::size_t num_nets{NUM_NETS};
  std::size_t num_blocks{NUM_BLOCKS};
  std::size_t num_cols{NUM_COLS};
  std::string block_name{BLOCK_NAME};
  std::string top_name{TOP_NAME};
  std::string block_prefix{BLOCK_PREFIX};
  std::string cell_prefix{CELL_PREFIX};
  std::string net_prefix{NET_PREFIX};
  std::string lib_cell_name{LIB_CELL_NAME};
  std::string lib_cell_inp_pin{LIB_CELL_INP_PIN};
  std::string lib_cell_out_pin{LIB_CELL_OUT_PIN};
  std::string lib_leaf_cell_name{LIB_LEAF_CELL_NAME};
  std::string lib_leaf_cell_d_pin{LIB_LEAF_CELL_D_PIN};
  double min_cap_val{MIN_CAP_VAL};
  double max_cap_val{MAX_CAP_VAL};
  std::size_t min_num_ccaps{MIN_NUM_CCAPS};
};

#endif  // DESIGN_CONFIG_HPP
