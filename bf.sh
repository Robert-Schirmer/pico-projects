#!/usr/bin/env bash

# Build flash
# This script builds the project and flashes it to the Raspberry Pi Pico
# Project builds are assumed to have a uf2 file in the build directory same name as the project directory
# Run this script from within the project directory ex. '../bf.sh'

# Assert command success
acs() {
  if ! "$@"; then
    exit 1
  fi
}

if [ ! -d "./build" ]; then
  acs cmake -S . -B ./build
fi

acs cmake --build ./build

parentdir="${PWD##*/}"

acs picotool load -f ./build/$parentdir.uf2
