#!/usr/bin/env bash

# Updates dev-smartmet branch of github_modules/smartmet-library-newbase and -smarttools 
# Update means 1) switch to dev, 2) pull github to dev, 3) push dev to github

dev=$(cat ./branches/dev_branch)

# First gis
cd ../../github_modules/smartmet-library-gis
git checkout $dev && git pull --rebase --no-edit origin $dev && git push --tags origin $dev

# Second newbase
cd ../../github_modules/smartmet-library-newbase
git checkout $dev && git pull --rebase --no-edit origin $dev && git push --tags origin $dev

# Third smarttools
cd ../../github_modules/smartmet-library-smarttools
git checkout $dev && git pull --rebase --no-edit origin $dev && git push --tags origin $dev

# 4th macgyver
cd ../../github_modules/smartmet-library-macgyver
git checkout $dev && git pull --rebase --no-edit origin $dev && git push --tags origin $dev

read -p "Press [Enter] key to continue..."
