#!/usr/bin/env bash

clang_format_exe_path="/d/Program Files (x86)/Microsoft Visual Studio/2019/Professional/VC/Tools/Llvm/bin/clang-format"

cd ../../github_modules

# First gis
gis_dir="smartmet-library-gis"
[ -d "$gis_dir" ] && "$clang_format_exe_path" -i -style=file -verbose "$gis_dir"/gis/*.cpp "$gis_dir"/gis/*.h

# Second newbase
newbase_dir="smartmet-library-newbase"
[ -d "$newbase_dir" ] && "$clang_format_exe_path" -i -style=file -verbose "$newbase_dir"/newbase/*.cpp "$newbase_dir"/newbase/*.h

# Third smarttools
smarttools_dir="smartmet-library-smarttools"
[ -d "$smarttools_dir" ] && "$clang_format_exe_path" -i -style=file -verbose "$smarttools_dir"/smarttools/*.cpp "$smarttools_dir"/smarttools/*.h

read -p "Press [Enter] key to continue..."
