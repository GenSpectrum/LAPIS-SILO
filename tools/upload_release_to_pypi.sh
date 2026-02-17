#!/bin/bash
set -euo pipefail

# Upload silodb wheels from a GitHub release to PyPI
#
# This script downloads all wheel files from a GitHub release and uploads
# them to PyPI in a single operation. This ensures all platform wheels are
# available simultaneously and avoids managing PyPI credentials in CI.
#
# Usage:
#   ./tools/upload_release_to_pypi.sh [VERSION]
#
# If VERSION is not specified, uses the version from version.txt
#
# Prerequisites:
#   - gh (GitHub CLI) - authenticated
#   - twine - pip install twine
#   - PyPI credentials configured (via ~/.pypirc or environment variables)

# Check that we're running from the repository root
if [ ! -f "pyproject.toml" ] || [ ! -f "version.txt" ]; then
    echo "ERROR: This script must be run from the repository root."
    echo "  cd to the repository root and run: ./tools/upload_release_to_pypi.sh"
    exit 1
fi

# Get version
if [ -n "${1:-}" ]; then
    VERSION="$1"
else
    VERSION=$(tr -d '[:space:]' < version.txt)
fi

TAG="v$VERSION"

echo "=== Uploading silodb $VERSION to PyPI ==="
echo ""

# Check if release exists
if ! gh release view "$TAG" &>/dev/null; then
    echo "ERROR: Release $TAG not found on GitHub."
    echo "  Available releases:"
    gh release list --limit 5
    exit 1
fi

# Create temporary directory for wheels
WHEEL_DIR=$(mktemp -d)
trap 'rm -rf \"$WHEEL_DIR\"' EXIT

echo "Step 1: Downloading wheels from GitHub release $TAG..."
gh release download "$TAG" --pattern "*.whl" --dir "$WHEEL_DIR"

# Check if any wheels were downloaded
WHEEL_COUNT=$(ls -1 "$WHEEL_DIR"/*.whl 2>/dev/null | wc -l)
if [ "$WHEEL_COUNT" -eq 0 ]; then
    echo "ERROR: No wheel files found in release $TAG"
    echo "  Make sure the Build Wheels workflow completed successfully."
    exit 1
fi

echo "  Downloaded $WHEEL_COUNT wheel(s):"
ls -1 "$WHEEL_DIR"/*.whl | sed 's/^/    /'
echo ""

# Check wheels with twine
echo "Step 2: Checking wheels with twine..."
twine check "$WHEEL_DIR"/*.whl
echo ""

# Show summary and confirm
echo "=== Ready to upload to PyPI ==="
echo "Version: $VERSION"
echo "Wheels:"
ls -lh "$WHEEL_DIR"/*.whl | awk '{print "  " $NF " (" $5 ")"}'
echo ""

read -p "Upload to PyPI? (y/N): " confirm
if [ "$confirm" != "y" ] && [ "$confirm" != "Y" ]; then
    echo "Aborted."
    exit 0
fi

# Upload to PyPI
echo ""
echo "Step 3: Uploading to PyPI..."
twine upload "$WHEEL_DIR"/*.whl

echo ""
echo "✓ Successfully uploaded silodb $VERSION to PyPI!"
echo ""
echo "Install with:"
echo "  pip install silodb==$VERSION"
