#!/usr/bin/env bash

dev=$(cat ./branches/dev_branch)

git checkout $dev && git pull --rebase --tags --no-edit origin $dev

read -p "Press [Enter] key to continue..."
