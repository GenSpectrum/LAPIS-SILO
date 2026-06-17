#!/bin/bash

set -euo pipefail

# Annotate a GitHub Release with a serialization version change note.
#
# Compares the serialization version file between the current release tag
# and the previous release tag. If it changed, inserts a warning note
# into the GitHub Release body using insert-serialization-version-note.sh.
#
# Usage:
#   ./annotate-release-serialization-version.sh --tag=<release-tag>
#
# Environment:
#   SERIALIZATION_VERSION_FILE  path inside the repo (default: src/silo/common/serialization_version.txt)
#   DRY_RUN                     if "true", print transformed body but skip gh release edit
#   GITHUB_TOKEN                required for gh release view/edit (unless DRY_RUN)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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

# Strip leading 'v' from tag to get numeric version for heading match
VERSION="${TAG#v}"

if [ "$DRY_RUN" = "true" ]; then
  echo "DRY_RUN: would update release $TAG body to:"
  echo "---"
  echo "(cannot fetch release body without GITHUB_TOKEN)"
  exit 0
fi

BODY=$(gh release view "$TAG" --json body -q .body)
NEW_BODY=$(echo "$BODY" | "${SCRIPT_DIR}/insert-serialization-version-note.sh" --version="$VERSION")

if [ "$BODY" = "$NEW_BODY" ]; then
  echo "Release body unchanged (note already present or heading not found), skipping"
  exit 0
fi

gh release edit "$TAG" --notes "$NEW_BODY"

echo "Annotated GitHub release $TAG with serialization version note"
