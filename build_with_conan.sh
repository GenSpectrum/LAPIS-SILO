#!/bin/bash

set -e

if [[ "$*" == *"clean"* ]]
then
  echo "cleaning build directory..."
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


if [[ "$*" == *"release"* ]]
then
  echo "triggered RELEASE build"
  cmake -D CMAKE_BUILD_TYPE=Release -B build
else
  if [[ "$*" == *"build_without_clang_tidy"* ]]
  then
    echo "triggered DEBUG build"
    cmake -D BUILD_WITH_CLANG_TIDY=OFF -B build
  else
    echo "triggered DEBUG build"
    cmake -B build
  fi
fi

echo "----------------------------------"
echo "cmake --build build"
echo "----------------------------------"

cmake --build build