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
    cmake -G Ninja -D BUILD_WITH_CLANG_TIDY=ON -D CMAKE_BUILD_TYPE=Debug -B build/Debug
    for file in $files; do
      echo "Checking file: $file"
      if [[ $file == src/*.cpp ]]; then
        echo "Now linting the file: $file"
        cmake --build build/Debug --target CMakeFiles/silolib.dir/${file}.o
      fi
    done'
