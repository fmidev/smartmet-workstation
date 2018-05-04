#!/usr/bin/env bash

cd ../../.. && mkdir -p smartmet_msvc && cd smartmet_msvc && cmake -G "Visual Studio 14 2015 Win64" ../SmartMet

read -p "Press [Enter] key to continue..."
