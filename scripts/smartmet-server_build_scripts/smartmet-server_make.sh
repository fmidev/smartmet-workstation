#!/bin/bash

# Check if argument is provided
if [ $# -eq 0 ]; then
    echo "Usage: $0 <make_command1> [<make_command2> ...]"
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

# Loop through modules and run make command
for index in "${!local_relative_base_directory[@]}"; do
    make_file_directory="${local_relative_base_directory[$index]}/${local_module_names[$index]}"
    make_file_path="$make_file_directory/Makefile"
    if [ -f "$make_file_path" ]; then
        echo "Running \"make $@\" in '$make_file_directory'"
        # Let's make following commands temporary in different directory
        (cd "$make_file_directory" && make "$@")
        echo "Finished \"make $@\" in '$make_file_directory'"
        echo
    else
        echo "Makefile not found in '$make_file_directory'"
    fi
done
