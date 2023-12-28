#!/bin/bash

# Check if argument is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <make_command>"
    exit 1
fi

file="fmi_modules.txt"
declare -a github_module_names
declare -a local_module_names
declare -a local_relative_base_directory

while IFS=' ' read -r str1 str2 str3 _rest; do
    # Skip comment lines and empty lines
    if [[ $str1 != "#"* && -n $str1 ]]; then
        github_module_names+=("$str1")
        local_module_names+=("$str2")
        local_relative_base_directory+=("$str3")
    fi
done < "$file"

# Define directories containing makefiles
directories=(
    "calculator"
    "delfoi"
    "fonts"
    "frontier"
    "gis"
    "giza"
    "grid"
    "grid-content"
    "grid-files"
    "imagine"
    "imagine2"
    "locus"
    "macgyver"
    "newbase"
    "press"
    "qdcontour"
    "qdcontour2"
    "regression"
    "roadindex"
    "roadmodel"
    "smarttools"
    "spine"
    "textgen"
    "timeseries"
    "timezones"
    "trajectory"
    "trax"
    "utils"
    "woml"
)

# Loop through modules and run make command
for index in "${!local_relative_base_directory[@]}"; do
    make_file_directory="${local_relative_base_directory[$index]}/${local_module_names[$index]}"
    make_file_path="$make_file_directory/Makefile"
    echo "Makefile path is $make_file_path"
    if [ -f "$make_file_path" ]; then
        echo "Running '$1' in '$make_file_directory'"
        (cd "$make_file_directory" && make "$1")
        echo "Finished '$1' in '$make_file_directory'"
        echo
    else
        echo "Makefile not found in '$make_file_directory'"
    fi
done
