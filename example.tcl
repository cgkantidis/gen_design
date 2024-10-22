set_app_var pt_tmp_dir /SCRATCH/gkan
set_app_var search_path /remote/dept5116c/testdata/libraries/
set_app_var link_path "* lsi_10k.db"
read_db lsi_10k.db
read_verilog ./block.v
read_verilog ./top.v
link_design top
set_app_var parasitics_read_in_foreground true
read_parasitics -format SPEF -path [get_cells] ./block.spef
read_parasitics -format SPEF ./top.spef
report_annotated_parasitics
