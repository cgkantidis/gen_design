set_app_var pt_tmp_dir /SCRATCH/gkan
set_app_var search_path /remote/dept5116c/testdata/libraries/
set_app_var link_path "* lsi_10k.db"
read_db lsi_10k.db
read_verilog /SCRATCH/gkan/top.v.gz
link top
