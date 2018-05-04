#!/usr/bin/env bash

mkdir -p git
mkdir -p cmake
cp -r ../tools/gitflow/* ./git/
cp ../tools/cmake_build_script/* ./cmake/
rm  ./git/LICENSE
rm ./git/README.md

