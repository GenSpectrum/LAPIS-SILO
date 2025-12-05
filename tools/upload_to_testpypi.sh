#!/bin/bash
set -e

# Check that we're running from the repository root
if [ ! -f "pyproject.toml" ] || [ ! -f "version.txt" ]; then
    echo "ERROR: This script must be run from the repository root."
    echo "  cd to the repository root and run: ./tools/upload_to_pypi.sh"
    exit 1
fi

# Check if current commit is tagged and matches version.txt
VERSION=$(cat version.txt | tr -d '[:space:]')
GIT_TAG=$(git describe --tags --exact-match 2>/dev/null || echo "")

if [ -z "$GIT_TAG" ]; then
    echo "WARNING: Current commit is not tagged."
    echo "  Version to upload: $VERSION"
    echo "  This script should only be run on release commits!"
    echo ""
    read -p "Continue anyway? (y/N): " confirm
    [ "$confirm" = "y" ] || [ "$confirm" = "Y" ] || exit 1
elif [ "$GIT_TAG" != "v$VERSION" ] && [ "$GIT_TAG" != "$VERSION" ]; then
    echo "WARNING: Git tag ($GIT_TAG) does not match version.txt ($VERSION)"
    echo "  This script should only be run on release commits!"
    echo ""
    read -p "Continue anyway? (y/N): " confirm
    [ "$confirm" = "y" ] || [ "$confirm" = "Y" ] || exit 1
else
    echo "Version check: $VERSION (tag: $GIT_TAG) ✓"
fi

echo "=== Building and uploading pysilo to PyPI ==="

# Check if wheelhouse already contains wheels
if [ -d "wheelhouse" ] && ls wheelhouse/*.whl &>/dev/null; then
    echo "WARNING: wheelhouse/ already contains wheels:"
    ls -1 wheelhouse/*.whl
    echo ""
    read -p "Clean wheelhouse before building? (Y/n): " clean_choice
    if [ "$clean_choice" != "n" ] && [ "$clean_choice" != "N" ]; then
        rm -f wheelhouse/*.whl
        echo "Cleaned wheelhouse/"
    fi
    echo ""
fi

# Step 1: Build wheel
echo "Step 1: Building wheel..."
python setup.py bdist_wheel

# Step 2: Install patchelf if needed
if ! command -v patchelf &> /dev/null; then
    echo "Step 2: Installing patchelf..."
    sudo apt-get install -y patchelf
else
    echo "Step 2: patchelf already installed ✓"
fi

# Step 3: Repair wheel for manylinux compatibility
echo "Step 3: Repairing wheel with auditwheel..."
mkdir -p wheelhouse

# Find the most recent pysilo wheel in dist/
WHEEL=$(ls -t dist/pysilo-*.whl 2>/dev/null | head -n 1)

if [ -z "$WHEEL" ]; then
    echo "ERROR: No wheel file found in dist/"
    exit 1
fi

echo "Found wheel: $WHEEL"
auditwheel repair "$WHEEL" -w wheelhouse/

# Step 4: Check the repaired wheel
echo "Step 4: Checking wheel..."
twine check wheelhouse/*

# Step 5: Show what will be uploaded
echo ""
echo "=== Ready to upload ==="
ls -lh wheelhouse/
echo ""

# Step 6: Upload
echo "Uploading to TestPyPI..."
twine upload --repository testpypi wheelhouse/*
echo ""
echo "✓ Uploaded to TestPyPI!"
echo "Test install with:"
echo "  pip install --index-url https://test.pypi.org/simple/ --extra-index-url https://pypi.org/simple/ pysilo"
