#!/bin/bash

set -euo pipefail

# Insert a "Serialization Version Changed" note into release markdown.
#
# Pure stdin/stdout text transformation — no git, no network, no files.
# Idempotent: passes input through unchanged if the note already exists.
#
# Usage:
#   cat CHANGELOG.md | ./insert-serialization-version-note.sh --version=<X.Y.Z>
#   gh release view ... | ./insert-serialization-version-note.sh --version=<X.Y.Z>
#
# The note is inserted directly after the "## [<version>]" heading line.

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

INPUT=$(cat)

# Guard: if note already present, pass through unchanged
if echo "$INPUT" | grep -q "### ⚠ Serialization Version Changed"; then
  echo "Serialization version note already present, skipping" >&2
  echo "$INPUT"
  exit 0
fi

# Insert note after the version heading
ESCAPED_VERSION="${VERSION//./\\.}"
echo "$INPUT" | awk -v ver="$ESCAPED_VERSION" '
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
'
