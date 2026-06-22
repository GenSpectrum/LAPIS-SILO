#!/bin/bash

set -euo pipefail

# Check whether the serialization version changed since the last release,
# and if so, annotate CHANGELOG.md and commit+push to the release branch.
#
# Delegates the actual text modification to insert-serialization-version-note.sh.
# Commits and pushes to the currently checked-out branch (HEAD).
#
# Usage:
#   ./annotate-changelog-serialization-version.sh --version=<release-version>
#
# Environment:
#   SERIALIZATION_VERSION_FILE  path inside the repo (default: src/silo/common/serialization_version.txt)
#   DRY_RUN                     if "true", skip commit+push (useful for local testing)

SCRIPT_DIR="$(unset CDPATH; cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

DRY_RUN="${DRY_RUN:-false}"

VERSION=""

for arg in "$@"; do
  case "$arg" in
    --version=*) VERSION="${arg#--version=}" ;;
    *) echo "Unknown argument: $arg" >&2; exit 1 ;;
  esac
done

if [ -z "$VERSION" ]; then
  echo "Usage: $0 --version=<release-version>" >&2
  exit 1
fi

echo "Release version: $VERSION"

git fetch --tags

PREV_TAG=$(git tag --sort=-version:refname | grep '^v' | head -1)
if [ -z "$PREV_TAG" ]; then
  echo "No previous release tag found, skipping"
  exit 0
fi

echo "Previous tag: $PREV_TAG"

if ! "${SCRIPT_DIR}/detect-serialization-version-change.sh" --old-ref="$PREV_TAG" --new-ref="origin/main"; then
  exit 0
fi

"${SCRIPT_DIR}/insert-serialization-version-note.sh" --version="$VERSION" \
  < CHANGELOG.md > CHANGELOG.md.tmp && mv CHANGELOG.md.tmp CHANGELOG.md

if [ "$DRY_RUN" = "true" ]; then
  echo "DRY_RUN: would commit and push CHANGELOG.md changes"
  exit 0
fi

git config user.name "github-actions[bot]"
git config user.email "41898282+github-actions[bot]@users.noreply.github.com"
git add CHANGELOG.md
git diff --cached --quiet && { echo "No changes to commit"; exit 0; }
git commit -m "chore: annotate changelog with serialization version change"
git push origin HEAD
