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

# Find the release tag immediately before $TAG in version order.
# Tags are listed newest-first; the tag after $TAG in this list is its predecessor.
PREV_TAG=$(git tag --sort=-version:refname | grep '^v' | awk -v tag="$TAG" 'found { print; exit } $0 == tag { found=1 }')
if [ -z "$PREV_TAG" ]; then
  echo "No previous release tag found, skipping"
  exit 0
fi

echo "Previous tag: $PREV_TAG"

if ! "${SCRIPT_DIR}/detect-serialization-version-change.sh" --old-ref="$PREV_TAG" --new-ref="$TAG"; then
  exit 0
fi

# Strip leading 'v' from tag to get numeric version for heading match
VERSION="${TAG#v}"

if [ "$DRY_RUN" = "true" ]; then
  echo "DRY_RUN: would update release $TAG body to:"
  echo "---"
  echo "(cannot fetch release body without GITHUB_TOKEN)"
  exit 0
fi

BODY=$(gh release view "$TAG" --json body -q '.body // ""')
NEW_BODY=$(echo "$BODY" | "${SCRIPT_DIR}/insert-serialization-version-note.sh" --version="$VERSION")

if [ "$BODY" = "$NEW_BODY" ]; then
  echo "Release body unchanged (note already present or heading not found), skipping"
  exit 0
fi

gh release edit "$TAG" --notes "$NEW_BODY"

echo "Annotated GitHub release $TAG with serialization version note"
