#!/usr/bin/env bash

# Get task branch's name from user
echo "Enter name for task branch:"
read name

# Remove whitespace
name="$(echo -e "${name}" | tr -d '[:space:]')"

# Store task branch's name
echo $name > ./branches/task_branch

# create branch for task and checkout
git branch $name && git checkout $name

read -p "Press [Enter] key to continue..."
