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

# Insert note after the version heading, scoped to that version's section.
# Idempotent: if the note already exists within the section, passes through unchanged.
ESCAPED_VERSION="${VERSION//./\\.}"
echo "$INPUT" | awk -v ver="$ESCAPED_VERSION" '
  # Detect next version heading (end of target section) — must run before
  # the start-section check so the heading line does not immediately close itself.
  in_section && /^## \[/ {
    in_section=0
  }
  # Match the target version heading
  /^## \[/ && $0 ~ "^## \\[" ver "\\]" && !found_version {
    found_version=1
    in_section=1
    version_line=NR
  }
  # If we find the note already in the section, mark as duplicate
  in_section && /^### ⚠ Serialization Version Changed/ {
    already_present=1
  }
  {lines[NR]=$0}
  END {
    if (already_present || !found_version) {
      # Pass through unchanged
      for (i=1; i<=NR; i++) print lines[i]
    } else {
      for (i=1; i<=NR; i++) {
        print lines[i]
        if (i == version_line) {
          print ""
          print "### ⚠ Serialization Version Changed"
          print ""
          print "The serialization version changed in this release. Databases serialized with previous SILO versions are incompatible and need to be re-preprocessed."
        }
      }
    }
  }
'
