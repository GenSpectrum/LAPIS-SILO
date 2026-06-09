#!/bin/bash

set -euo pipefail

# Annotate CHANGELOG.md with a serialization version change note.
#
# Compares the serialization version file between the latest release tag
# and origin/main. If it changed, inserts a warning section into the
# CHANGELOG.md entry for the given release version and commits+pushes
# the change.
#
# Usage:
#   ./annotate-changelog-serialization-version.sh --branch=<release-branch> --version=<release-version>
#
# Environment:
#   SERIALIZATION_VERSION_FILE  path inside the repo (default: src/silo/common/serialization_version.txt)
#   DRY_RUN                     if "true", skip commit+push (useful for local testing)

SERIALIZATION_VERSION_FILE="${SERIALIZATION_VERSION_FILE:-src/silo/common/serialization_version.txt}"
DRY_RUN="${DRY_RUN:-false}"

BRANCH=""
VERSION=""

for arg in "$@"; do
  case "$arg" in
    --branch=*) BRANCH="${arg#--branch=}" ;;
    --version=*) VERSION="${arg#--version=}" ;;
    *) echo "Unknown argument: $arg"; exit 1 ;;
  esac
done

if [ -z "$BRANCH" ] || [ -z "$VERSION" ]; then
  echo "Usage: $0 --branch=<release-branch> --version=<release-version>"
  exit 1
fi

echo "Release PR branch: $BRANCH, version: $VERSION"

git fetch origin "$BRANCH"
git checkout -B "$BRANCH" "origin/$BRANCH"
git fetch --tags

PREV_TAG=$(git tag --sort=-version:refname | grep '^v' | head -1)
if [ -z "$PREV_TAG" ]; then
  echo "No previous release tag found, skipping"
  exit 0
fi

echo "Previous tag: $PREV_TAG"

OLD_VER=$(git show "${PREV_TAG}:${SERIALIZATION_VERSION_FILE}" 2>/dev/null || echo "unknown")
NEW_VER=$(git show "origin/main:${SERIALIZATION_VERSION_FILE}" 2>/dev/null || echo "unknown")

if [ "$OLD_VER" = "$NEW_VER" ]; then
  echo "Serialization version unchanged ($OLD_VER), skipping"
  exit 0
fi

echo "Serialization version changed: $OLD_VER -> $NEW_VER"

# Guard: skip if note already present in CHANGELOG.md for this version
if grep -q "### ⚠ Serialization Version Changed" CHANGELOG.md; then
  echo "Serialization version note already present in CHANGELOG.md, skipping"
  exit 0
fi

# Insert note after the version heading for this release.
ESCAPED_VERSION="${VERSION//./\\.}"
awk -v ver="$ESCAPED_VERSION" '
  /^## \[/ && $0 ~ "^## \\[" ver "\\]" && !done {
    print
    print ""
    print "### ⚠ Serialization Version Changed"
    print ""
    print "The serialization version changed in this release. Databases serialized with previous SILO versions are incompatible and need to be re-preprocessed."
    done=1
    next
  }
  {print}
' CHANGELOG.md > CHANGELOG.md.tmp && mv CHANGELOG.md.tmp CHANGELOG.md

if [ "$DRY_RUN" = "true" ]; then
  echo "DRY_RUN: would commit and push CHANGELOG.md changes"
  exit 0
fi

git config user.name "github-actions[bot]"
git config user.email "41898282+github-actions[bot]@users.noreply.github.com"
git add CHANGELOG.md
git diff --cached --quiet && { echo "No changes to commit"; exit 0; }
git commit -m "chore: annotate changelog with serialization version change"
git push origin "$BRANCH"
