#!/bin/bash

set -e

if [[ "$*" == *"clean"* ]]
then
  echo "cleaning build directory..."
  rm -rf build
fi
mkdir -p build

echo "----------------------------------"
echo "cmake -B build"
echo "----------------------------------"


if [[ "$*" == *"release"* ]]
then
  echo "triggered RELEASE build"
  conan install . --build=missing --profile ./conanprofile --profile:build ./conanprofile --output-folder=build
  cmake -D CMAKE_BUILD_TYPE=Release -B build
else
  echo "triggered DEBUG build"
  conan install . --build=missing --profile ./conanprofile --profile:build ./conanprofile --output-folder=build -s build_type=Debug

  if [[ "$*" == *"build_without_clang_tidy"* ]]
  then
    cmake -D BUILD_WITH_CLANG_TIDY=OFF -B build
  else
    echo "... with clang-tidy"
    cmake -B build
  fi
fi

echo "----------------------------------"
echo "cmake --build build"
echo "----------------------------------"

cmake --build build --parallel