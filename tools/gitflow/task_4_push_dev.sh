#!/usr/bin/env bash

dev=$(cat ./branches/dev_branch)

git checkout $dev && git push --tags origin $dev

read -p "Press [Enter] key to continue..."
