#!/usr/bin/env bash

task=$(cat ./branches/task_branch)
dev=$(cat ./branches/dev_branch)

git checkout $dev && git merge --no-edit --no-ff $task

read -p "Press [Enter] key to continue..."
