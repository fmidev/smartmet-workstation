#!/usr/bin/env bash

BaseProjectDir="smartmet-workstation-wgs84"

cd ../../.. && mkdir -p ${BaseProjectDir}_msvc && cd ${BaseProjectDir}_msvc && cmake -G "Visual Studio 16 2019" -A x64 ../$BaseProjectDir

read -p "Press [Enter] key to continue..."
