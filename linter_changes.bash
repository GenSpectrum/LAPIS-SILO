#!/bin/bash

# Exit on any error
set -euo pipefail

# Check if the image argument is provided, otherwise use :latest
DOCKER_IMAGE="${1:-ghcr.io/genspectrum/lapis-silo-dependencies:latest}"

echo "Using Docker image: $DOCKER_IMAGE"
docker pull "$DOCKER_IMAGE"

# Get the list of changed files from the local Git repository, only (A)dded, (C)reated, and (M)odified
files=$(git diff --name-only --diff-filter=ACM origin/main)

# Check if any files were found
if [[ -z "$files" ]]; then
  echo "No changed files found. Exiting."
  exit 0
fi

# Iterate over the files and run clang-tidy on .cpp files
echo "Changed files:"
echo "$files"
docker run --rm \
  -v "$(pwd)/.clang-tidy:/src/.clang-tidy" \
  -v "$(pwd)/performance:/src/performance" \
  -v "$(pwd)/src:/src/src" \
  -v "$(pwd)/CMakeLists.txt:/src/CMakeLists.txt" \
  -w /src \
  -e files="$files" \
  "$DOCKER_IMAGE" \
  /bin/bash -c '
    set -euo pipefail
    cmake -G Ninja -D BUILD_WITH_CLANG_TIDY=ON -D CMAKE_BUILD_TYPE=Debug -B build/Debug
    targets=()
    for file in $files; do
      echo "Considering to lint: $file"
      if [[ $file == src/*.test.cpp ]]; then
        targets+=(--target "CMakeFiles/silo_test.dir/${file}.o")
      elif [[ $file == src/*.cpp ]]; then
        targets+=(--target "CMakeFiles/silolib.dir/${file}.o")
      fi
    done
    echo "Collected targets: ${targets[@]}"
    if [[ ${#targets[@]} -gt 0 ]]; then
      cmake --build build/Debug "${targets[@]}" --parallel 16
    fi'
