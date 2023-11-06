#!/usr/bin/env bash

BaseProjectDir="smartmet-workstation"

cd ../../.. && mkdir -p ${BaseProjectDir}_msvc && cd ${BaseProjectDir}_msvc && cmake -G "Visual Studio 16 2019" -A x64 ../$BaseProjectDir
# cd ../../.. && mkdir -p ${BaseProjectDir}_msvc && cd ${BaseProjectDir}_msvc && cmake -G "Visual Studio 16 2019" -A x64 ../$BaseProjectDir --graphviz=foo.dot

read -p "Press [Enter] key to continue..."
