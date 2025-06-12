#!/bin/bash

# Exit on any error
set -e

# Check if the image argument is provided, otherwise use :latest
DOCKER_IMAGE="${1:-ghcr.io/genspectrum/lapis-silo-dependencies:latest}"

echo "Using Docker image: $DOCKER_IMAGE"

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
  -v "$(pwd)/src:/src/src" \
  -v "$(pwd)/CMakeLists.txt:/src/CMakeLists.txt" \
  -w /src \
  -e files="$files" \
  "$DOCKER_IMAGE" \
  /bin/bash -c '
    cmake -D BUILD_WITH_CLANG_TIDY=ON -D CMAKE_BUILD_TYPE=Debug -B build/Debug
    for file in $files; do
      echo "Checking file: $file"
      if [[ $file == *.cpp ]]; then
        echo "Now linting the file: $file"
        target=$(basename "${file%.cpp}.o")
        cmake --build build/Debug --target ${file%.cpp}.o
      fi
    done'
