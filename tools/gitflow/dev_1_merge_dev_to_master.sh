#!/usr/bin/env bash

dev=$(cat ./branches/dev_branch)

master=$(cat ./branches/master_branch)

git checkout $master && git merge --no-edit --no-ff $dev

read -p "Press [Enter] key to continue..."
