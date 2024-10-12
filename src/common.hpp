#include <string>

// the design will have `num_blocks * num_nets` nets by the end, and
// `num_blocks * num_nets + num_blocks` net segments
// 1'000'000 x 4'500 = 4.5B, which is a little larger than 2^32
static constexpr std::size_t num_nets{1'000'000};
static constexpr std::size_t num_blocks{4'500};
static constexpr std::size_t num_cols{80};
static constexpr std::string block_name{"block"};
static constexpr std::string top_name{"top"};
static constexpr std::string block_prefix{"b"};
static constexpr std::string cell_prefix{"u"};
static constexpr std::string net_prefix{"n"};
static constexpr std::string lib_cell_name{"IV"};
static constexpr std::string lib_cell_inp_pin{"A"};
static constexpr std::string lib_cell_out_pin{"Z"};
