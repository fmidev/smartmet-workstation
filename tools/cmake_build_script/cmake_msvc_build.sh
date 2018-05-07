#!/usr/bin/env bash

BaseProjectDir="smartmet-workstation"

cd ../../.. && mkdir -p ${BaseProjectDir}_msvc && cd ${BaseProjectDir}_msvc && cmake -G "Visual Studio 14 2015 Win64" ../$BaseProjectDir

read -p "Press [Enter] key to continue..."
