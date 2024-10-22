# Build

## Writing uncompressed files

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

## Writing compressed files (slower but takes less disk space)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DWRITE_COMPRESSED=ON
cmake --build build -j$(nproc)
```

# Run

## Getting Help

```bash
build/gen_design --help
# Generate the necessary verilog and SPEF files for a design
# Usage:
#   gen_design [OPTION...]
# 
#   -n, --num_nets arg    The number of nets in each block hierarchy
#   -b, --num_blocks arg  The number of block hierarchies in the top
#                         hierarchy
#   -h, --help            Print this help message
```

## Generating design files

To generate a design with 4.5B nets

```bash
build/gen_design -n 1000000 -b 4500
```

