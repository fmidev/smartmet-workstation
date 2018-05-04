#!/usr/bin/env bash

master=$(cat ./branches/master_branch)

git checkout $master && git pull --no-edit origin $master

read -p "Press [Enter] key to continue..."
