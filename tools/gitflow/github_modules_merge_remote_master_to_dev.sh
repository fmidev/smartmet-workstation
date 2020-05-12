#!/usr/bin/env bash

# Does merges remote master to local dev for github_modules/smartmet-library-newbase and -smarttools 
# This means 1) switch to dev, 4) merge from remote master

dev=$(cat ./branches/dev_branch)

master=$(cat ./branches/master_branch)

# First gis
cd ../../github_modules/smartmet-library-gis
git checkout $dev && git merge --no-edit --no-ff origin $master

# Second newbase
cd ../../github_modules/smartmet-library-newbase
git checkout $dev && git merge --no-edit --no-ff origin $master

# Third smarttools
cd ../../github_modules/smartmet-library-smarttools
git checkout $dev && git merge --no-edit --no-ff origin $master

read -p "Press [Enter] key to continue..."
