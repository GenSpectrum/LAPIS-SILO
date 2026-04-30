# Review: `src/silo/query_engine/saneql/ast_to_query.cpp`

## Bugs

L128-131: 🔴 bug: `LESS_THAN` and `LESS_EQUAL` both produce identical `FloatBetween(col, nullopt, value)`. `FloatBetween::compile` uses `Comparator::LESS` (strict `<`), so `x <= 5.0` silently becomes `x < 5.0`. Same issue L133-136: `GREATER_THAN` and `GREATER_EQUAL` both produce `FloatBetween(col, value, nullopt)` which compiles to `>=`, so `x > 5.0` becomes `x >= 5.0`. Fix: either add a `bool inclusive` flag to `FloatBetween`, or use epsilon adjustment, or add separate `FloatLessThan`/`FloatLessEqual` expressions.

L107: 🔴 bug: `value > 0 ? std::optional<uint32_t>(value - 1) : 0` — when `value == 0`, the ternary returns bare `0` (an `int`), not `std::optional<uint32_t>(0)`. This means `x < 0` on unsigned produces `IntBetween(col, nullopt, 0)` which matches rows where `col == 0` — should match nothing. Fix: return `std::nullopt` and wrap in a `False` expression, or return `IntBetween(col, nullopt, std::optional<uint32_t>{})` with both bounds empty then intersect with False.

L112-113: 🔴 bug: `GREATER_THAN` computes `value + 1` — unsigned overflow when `value == UINT32_MAX`. Produces `IntBetween(col, 0, nullopt)` which matches everything. Fix: check for `UINT32_MAX` and return `False` expression.

## Narrowing casts (int64_t → uint32_t without validation)

L74: 🟡 risk: `static_cast<uint32_t>(extractIntLiteral(value_expr))` — `extractIntLiteral` returns `int64_t`. Negative values or values > UINT32_MAX silently truncate. Fix: add range check + `IllegalQueryException`.

L184: 🟡 risk: same pattern in `convertComparisonToFilter`.

L284,287: 🟡 risk: same pattern in `handleBetween`.

L376,401,411: 🟡 risk: same pattern in `handleSymbolEquals`, `handleHasMutation`, `handleInsertionContains`.

L433: 🟡 risk: `static_cast<int>(extractIntLiteral(...))` for `nOf` count — `int64_t` → `int` truncation. Negative count also not validated.

L737: 🟡 risk: same pattern in `handleLimit`.

**Recommendation:** Extract a helper like `extractUint32Literal(expr, param_name)` that validates range `[0, UINT32_MAX]` and throws `IllegalQueryException` on out-of-range. Use everywhere. `getOptionalUint32` (function_registry.cpp:49) already checks `>= 0` but not `<= UINT32_MAX` — fix that too.

## Design

L471-472: ❓ q: bare identifier in filter context becomes `BoolEquals(name, true)`. Intentional? If user writes `.filter(some_column)` where `some_column` is a string column, they get a confusing runtime error instead of a clear "expected boolean expression" message. Consider checking column type or at least documenting this behavior.

L680,701: 🟡 risk: `handleMutations` and `handleInsertions` dispatch `Nucleotide` vs `AminoAcid` via `args.functionName() == "mutations"` / `"insertions"` string comparison. Fragile — rename the registered function and this silently breaks. Fix: register separate handlers, or use an enum/tag passed at registration time.

L723: 🔵 nit: `static_cast<uint32_t>(std::chrono::system_clock::now().time_since_epoch().count())` — truncates a 64-bit nanosecond count to 32 bits. Works as a seed but loses entropy. Consider `std::random_device` or at least cast from `steady_clock` which is monotonic.

## Edge cases

L152: 🟡 risk: `date_val.value() - 1` for `LESS_THAN` on dates — no underflow check. If `date_val` is the minimum representable date, this wraps. Same L160 for `date_val.value() + 1` overflow.

L377: Good — `position > 0` check exists for 1-indexed positions. But L411 (`handleInsertionContains`) has no such check — insertions may be 0-indexed but this should be documented or validated consistently.

## Style

L12-14: 🔵 nit: include order — `parser.h` (internal) appears before `<fmt/format.h>` and `<re2/re2.h>` (external). Per AGENTS.md include order: corresponding header → system → external → internal.

L958-959: 🔵 nit: lines exceed 100-char column limit (clang-format should catch this).

## Good practices

- Registry pattern clean and extensible — adding new functions is one line each.
- `BoundArguments` abstraction keeps handlers focused on semantics.
- Error messages include source locations — good for user-facing diagnostics.
- `CHECK_SILO_QUERY` used consistently for validation.
