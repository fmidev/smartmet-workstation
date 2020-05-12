#!/usr/bin/env bash

# Does initial git clone for github_modules/smartmet-library-newbase and -smarttools 
# This means 1) if directory exist, do nothing, 2) git clone, 3) switch to dev, 4) pull github to dev

dev=$(cat ./branches/dev_branch)

master=$(cat ./branches/master_branch)

cd ../..
mkdir -p github_modules
cd github_modules

# First gis
gis_dir="smartmet-library-gis"
[ ! -d "$gis_dir" ] && git clone git@github.com:fmidev/"$gis_dir".git && cd "$gis_dir" && git checkout $dev && git pull --no-edit origin $dev && cd ..

# Second newbase
newbase_dir="smartmet-library-newbase"
[ ! -d "$newbase_dir" ] && git clone git@github.com:fmidev/"$newbase_dir".git && cd "$newbase_dir" && git checkout $dev && git pull --no-edit origin $dev && cd ..

# Third smarttools
smarttools_dir="smartmet-library-smarttools"
[ ! -d "$smarttools_dir" ] && git clone git@github.com:fmidev/"$smarttools_dir".git && cd "$smarttools_dir" && git checkout $dev && git pull --no-edit origin $dev && cd ..

read -p "Press [Enter] key to continue..."
