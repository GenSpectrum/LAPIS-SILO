#!/bin/bash

set -e

if [ "$1" == "clean" ]
then
  rm -rf build
fi
mkdir -p build

echo "----------------------------------"
echo "conan install"
echo "----------------------------------"

conan install . --build=missing --profile ./conanprofile -if=build

echo "----------------------------------"
echo "cmake -B build"
echo "----------------------------------"

cmake -B build

echo "----------------------------------"
echo "cmake --build build"
echo "----------------------------------"

cmake --build build