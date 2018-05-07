#!/usr/bin/env bash

dev=$(cat ./branches/dev_branch)

master=$(cat ./branches/master_branch)

git rev-parse --verify $dev
if [ $? != 0 ]
then
git branch $dev
fi

git checkout $dev && git merge --no-edit --no-ff $master

read -p "Press [Enter] key to continue..."
