#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCRIPT="${SCRIPT_DIR}/insert-serialization-version-note.sh"

PASSED=0
FAILED=0

check() {
  local name="$1" expected="$2" actual="$3"
  if [ "$expected" = "$actual" ]; then
    echo "  PASS: $name"
    PASSED=$((PASSED + 1))
  else
    echo "  FAIL: $name"
    diff <(echo "$expected") <(echo "$actual") || true
    FAILED=$((FAILED + 1))
  fi
}

# --- Test 1: Normal insertion ---
echo "Test 1: Normal insertion"
INPUT='## [0.5.0](url)

### Features
* y

## [0.4.0](url)

### Features
* x'

EXPECTED='## [0.5.0](url)

### ⚠ Serialization Version Changed

The serialization version changed in this release. Databases serialized with previous SILO versions are incompatible and need to be re-preprocessed.

### Features
* y

## [0.4.0](url)

### Features
* x'

OUTPUT=$(echo "$INPUT" | "$SCRIPT" --version=0.5.0)
check "insert after 0.5.0 heading" "$EXPECTED" "$OUTPUT"

# --- Test 2: Idempotency — note already in target section ---
echo "Test 2: Idempotency (note already in 0.5.0 section)"
INPUT='## [0.5.0](url)

### ⚠ Serialization Version Changed

Note.

### Features
* y'

OUTPUT=$(echo "$INPUT" | "$SCRIPT" --version=0.5.0)
check "no duplicate note" "$INPUT" "$OUTPUT"

# --- Test 3: Note in older version only — should still insert into new ---
echo "Test 3: Scoped guard (note in 0.4.0, insert into 0.5.0)"
INPUT='## [0.5.0](url)

### Features
* y

## [0.4.0](url)

### ⚠ Serialization Version Changed

Note.

### Features
* x'

EXPECTED='## [0.5.0](url)

### ⚠ Serialization Version Changed

The serialization version changed in this release. Databases serialized with previous SILO versions are incompatible and need to be re-preprocessed.

### Features
* y

## [0.4.0](url)

### ⚠ Serialization Version Changed

Note.

### Features
* x'

OUTPUT=$(echo "$INPUT" | "$SCRIPT" --version=0.5.0)
check "insert into 0.5.0, preserve 0.4.0 note" "$EXPECTED" "$OUTPUT"

# --- Test 4: Single section (release body format) ---
echo "Test 4: Release body format"
INPUT='## [0.11.3](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.11.2...v0.11.3) (2026-05-18)


### Features

* feature ([abc](url))'

EXPECTED='## [0.11.3](https://github.com/GenSpectrum/LAPIS-SILO/compare/v0.11.2...v0.11.3) (2026-05-18)

### ⚠ Serialization Version Changed

The serialization version changed in this release. Databases serialized with previous SILO versions are incompatible and need to be re-preprocessed.


### Features

* feature ([abc](url))'

OUTPUT=$(echo "$INPUT" | "$SCRIPT" --version=0.11.3)
check "insert into release body" "$EXPECTED" "$OUTPUT"

# --- Test 5: No matching version — passthrough ---
echo "Test 5: No matching version heading"
INPUT='## [0.5.0](url)

### Features
* y'

OUTPUT=$(echo "$INPUT" | "$SCRIPT" --version=0.9.0)
check "passthrough unchanged" "$INPUT" "$OUTPUT"

# --- Summary ---
echo ""
echo "Results: $PASSED passed, $FAILED failed"
if [ "$FAILED" -gt 0 ]; then
  exit 1
fi
