#!/usr/bin/env bash
# Bumps the SILO serialization version to a new Unix timestamp and rebuilds the committed
# serialized test state so that unit tests pass with the new version.
#
# Usage: ./bump_serialization_version.sh

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SERIALIZATION_VERSION_FILE="$REPO_ROOT/src/silo/common/serialization_version.txt"
SERIALIZED_STATE_DIR="$REPO_ROOT/testBaseData/siloSerializedState"

NEW_VERSION="$(date +%s)"

echo "New serialization version: $NEW_VERSION"

# 1. Update the serialization version file
printf '%s\n' "$NEW_VERSION" > "$SERIALIZATION_VERSION_FILE"
echo "Updated $SERIALIZATION_VERSION_FILE"

# 2. Remove old serialized state directories
echo "Removing old serialized state directories..."
find "$SERIALIZED_STATE_DIR" -mindepth 1 -maxdepth 1 -type d -exec rm -rf {} +

# 3. Build the test binary
echo "Building silo_test..."
make -C "$REPO_ROOT" build/Debug/silo_test

# 4. Run the save/reload test with SILO_KEEP_SERIALIZED_STATE to preserve the new state
echo "Generating new serialized state..."
SILO_KEEP_SERIALIZED_STATE=1 "$REPO_ROOT/build/Debug/silo_test" \
    --gtest_filter="DatabaseTest.shouldSaveAndReloadDatabaseWithoutErrors"

# 5. Verify a new directory was created
NEW_DIRS=$(find "$SERIALIZED_STATE_DIR" -mindepth 1 -maxdepth 1 -type d)
if [ -z "$NEW_DIRS" ]; then
    echo "ERROR: No new serialized state directory was created." >&2
    exit 1
fi

echo ""
echo "Done. New serialized state directory:"
echo "$NEW_DIRS"
echo ""
echo "Remember to 'git add' the new directory and commit."
