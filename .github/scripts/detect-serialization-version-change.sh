#!/bin/bash

set -euo pipefail

# Detect whether the serialization version changed between two git refs.
#
# Exits 0 if the version changed, exits 1 if unchanged (or if either ref
# is missing the file). This convention allows callers to use:
#
#   if ./detect-serialization-version-change.sh --old-ref=v1.0.0 --new-ref=origin/main; then
#     echo "changed"
#   fi
#
# Usage:
#   ./detect-serialization-version-change.sh --old-ref=<git-ref> --new-ref=<git-ref>
#
# Environment:
#   SERIALIZATION_VERSION_FILE  path inside the repo (default: src/silo/common/serialization_version.txt)

SERIALIZATION_VERSION_FILE="${SERIALIZATION_VERSION_FILE:-src/silo/common/serialization_version.txt}"

OLD_REF=""
NEW_REF=""

for arg in "$@"; do
  case "$arg" in
    --old-ref=*) OLD_REF="${arg#--old-ref=}" ;;
    --new-ref=*) NEW_REF="${arg#--new-ref=}" ;;
    *) echo "Unknown argument: $arg" >&2; exit 1 ;;
  esac
done

if [ -z "$OLD_REF" ] || [ -z "$NEW_REF" ]; then
  echo "Usage: $0 --old-ref=<git-ref> --new-ref=<git-ref>" >&2
  exit 1
fi

OLD_VER=$(git show "${OLD_REF}:${SERIALIZATION_VERSION_FILE}" 2>/dev/null) || {
  echo "Serialization version file not found at ref $OLD_REF, skipping"
  exit 1
}
NEW_VER=$(git show "${NEW_REF}:${SERIALIZATION_VERSION_FILE}" 2>/dev/null) || {
  echo "Serialization version file not found at ref $NEW_REF, skipping"
  exit 1
}

if [ "$OLD_VER" = "$NEW_VER" ]; then
  echo "Serialization version unchanged ($OLD_VER)"
  exit 1
fi

echo "Serialization version changed: $OLD_VER -> $NEW_VER"
exit 0
