# AGENTS.md - Developer Guide for LAPIS-SILO

C++23 high-performance genomic sequence indexing engine. Uses Arrow Acero for query execution, roaring bitmaps for filtering, SaneQL as query language.

**Key Technologies:** C++23, CMake, Conan, GTest, spdlog, Arrow Acero, Boost, simdjson

## Build & Test

```bash
# Initial setup (once)
conan profile detect && conan profile show --context build > conanprofile

# Build
make build/Debug/silo              # Debug build (includes clang-tidy + ASAN)
make build/Release/silo            # Release build (includes mimalloc on Linux)

# Test
make test                          # Build and run C++ unit tests
make all-tests                     # Run C++, Python, and E2E tests

# Run specific tests
build/Debug/silo_test --gtest_filter='TestSuite*'         # By suite
build/Debug/silo_test --gtest_filter='TestSuite.testCase'  # By case
build/Debug/silo_test --gtest_filter='*Pattern*'           # By pattern

# Format & CI
make format                        # Format C++ code (requires clang-format)
make ci                            # Format + run all tests
```

CMake auto-scans `src/` directories. Adding/removing `.cpp` or `.h` files requires no CMakeLists.txt changes, but does require a clean rebuild (`make full-clean`).

## Code Style

### Naming (enforced by clang-tidy)
- **Files:** `snake_case.cpp`, `snake_case.h`
- **Namespaces:** `lower_case`
- **Classes/Structs:** `CamelCase`
- **Functions/Methods:** `camelBack`
- **Variables/Members:** `lower_case`
- **Constants/Enums:** `UPPER_CASE`

### Formatting (clang-format 17)
- 3-space indent, 100-char column limit, Chromium brace style, `#pragma once`

### Include order
```cpp
#include "silo/my_module/my_file.h"     // 1. Corresponding header

#include <string>                        // 2. System (standard library)

#include <arrow/acero/exec_plan.h>       // 3. External (angle brackets)

#include "silo/common/panic.h"           // 4. Internal (quotes)
```

### Error handling
- `std::expected<T, Error>` for recoverable errors.
- `SILO_ASSERT()` for debug assertions.
- Arrow functions: use `ARROW_ASSIGN_OR_RAISE` / `ARROW_RETURN_NOT_OK` macros.
- `CHECK_SILO_QUERY(condition, fmt, args...)` for query validation errors shown to users.
- Exceptions only for preprocessing/initialization.

### Logging
Use spdlog macros: `SPDLOG_ERROR`, `SPDLOG_INFO`, `SPDLOG_DEBUG`, `SPDLOG_TRACE`.
Runtime level controlled by `SPDLOG_LEVEL` env var (`trace`, `debug`, `info`, `warn`, `error`, `off`).

## Commit Messages

[Conventional Commits](https://www.conventionalcommits.org/). Must reference an issue.

```
feat: add unionAll operator

resolves #1221
```

**Types:** `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`
