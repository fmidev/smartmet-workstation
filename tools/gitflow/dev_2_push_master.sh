#!/usr/bin/env bash

master=$(cat ./branches/master_branch)

git checkout $master && git push --tags origin $master

read -p "Press [Enter] key to continue..."
