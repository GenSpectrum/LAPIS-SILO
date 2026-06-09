#!/bin/bash

set -euo pipefail

# Annotate a GitHub Release with a serialization version change note.
#
# Compares the serialization version file between the current release tag
# and the previous release tag. If it changed, appends a warning section
# to the GitHub Release body.
#
# Usage:
#   ./annotate-release-serialization-version.sh --tag=<release-tag>
#
# Environment:
#   SERIALIZATION_VERSION_FILE  path inside the repo (default: src/silo/common/serialization_version.txt)
#   DRY_RUN                     if "true", print what would be appended but skip gh release edit
#   GITHUB_TOKEN                required for gh release view/edit (unless DRY_RUN)

SERIALIZATION_VERSION_FILE="${SERIALIZATION_VERSION_FILE:-src/silo/common/serialization_version.txt}"
DRY_RUN="${DRY_RUN:-false}"

TAG=""

for arg in "$@"; do
  case "$arg" in
    --tag=*) TAG="${arg#--tag=}" ;;
    *) echo "Unknown argument: $arg"; exit 1 ;;
  esac
done

if [ -z "$TAG" ]; then
  echo "Usage: $0 --tag=<release-tag>"
  exit 1
fi

echo "Release tag: $TAG"

git fetch --tags

# Find the previous release tag (second newest v* tag)
PREV_TAG=$(git tag --sort=-version:refname | grep '^v' | sed -n '2p')
if [ -z "$PREV_TAG" ]; then
  echo "No previous release tag found, skipping"
  exit 0
fi

echo "Previous tag: $PREV_TAG"

OLD_VER=$(git show "${PREV_TAG}:${SERIALIZATION_VERSION_FILE}" 2>/dev/null || echo "unknown")
NEW_VER=$(git show "${TAG}:${SERIALIZATION_VERSION_FILE}" 2>/dev/null || echo "unknown")

if [ "$OLD_VER" = "$NEW_VER" ]; then
  echo "Serialization version unchanged ($OLD_VER), skipping"
  exit 0
fi

echo "Serialization version changed: $OLD_VER -> $NEW_VER"

NOTE=$'\n---\n\n### ⚠ Serialization Version Changed\n\nThe serialization version changed in this release. Databases serialized with previous SILO versions are incompatible and need to be re-preprocessed.'

if [ "$DRY_RUN" = "true" ]; then
  echo "DRY_RUN: would append to release $TAG:"
  echo "$NOTE"
  exit 0
fi

BODY=$(gh release view "$TAG" --json body -q .body)

# Guard: skip if note already present in release body
if echo "$BODY" | grep -q "### ⚠ Serialization Version Changed"; then
  echo "Serialization version note already present in release $TAG, skipping"
  exit 0
fi

gh release edit "$TAG" --notes "${BODY}${NOTE}"

echo "Annotated GitHub release $TAG with serialization version note"
