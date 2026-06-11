#!/bin/bash

set -euo pipefail

# Insert a "Serialization Version Changed" note into a CHANGELOG.md file.
#
# Pure text transformation — no git, no network. Modifies the file in place.
# Idempotent: exits successfully without changes if the note already exists.
#
# Usage:
#   ./insert-serialization-version-note.sh --version=<release-version> --changelog=<path>
#
# The note is inserted directly after the "## [<version>]" heading.

VERSION=""
CHANGELOG="CHANGELOG.md"

for arg in "$@"; do
  case "$arg" in
    --version=*) VERSION="${arg#--version=}" ;;
    --changelog=*) CHANGELOG="${arg#--changelog=}" ;;
    *) echo "Unknown argument: $arg"; exit 1 ;;
  esac
done

if [ -z "$VERSION" ]; then
  echo "Usage: $0 --version=<release-version> [--changelog=<path>]"
  exit 1
fi

if [ ! -f "$CHANGELOG" ]; then
  echo "Changelog file not found: $CHANGELOG"
  exit 1
fi

# Guard: skip if note already present
if grep -q "### ⚠ Serialization Version Changed" "$CHANGELOG"; then
  echo "Serialization version note already present in $CHANGELOG, skipping"
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
' "$CHANGELOG" > "${CHANGELOG}.tmp" && mv "${CHANGELOG}.tmp" "$CHANGELOG"

echo "Inserted serialization version note into $CHANGELOG for version $VERSION"
