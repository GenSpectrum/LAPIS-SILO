# PR #1238 — Parser Review (`parser.cpp` / `parser.h`)

Overall: clean recursive descent parser, well-structured, good test coverage. Findings below.

---

## Bugs / Risks

`parser.cpp:L243-L250`: 🟡 risk: `parseSetOrRecordExpression` disambiguation is fragile. When first token is IDENTIFIER, it calls `parseExpression()` which can consume far beyond the identifier (e.g. `{a.foo() := 1}` — `parseExpression` parses `a.foo()` which is a FunctionCall, not an Identifier, so `holds_alternative<ast::Identifier>` is false → falls through to SetLiteral path → `:=` becomes trailing garbage → error). This is correct behavior for rejecting bad input, but the error message will be confusing ("Expected RightBrace but got ColonEquals" or similar). Consider: if token after IDENTIFIER is `:=`, peek directly at the two tokens instead of parsing a full expression first. Or: add a targeted error message when `:=` follows a non-identifier expression inside `{}`.

`parser.cpp:L348-L357`: 🟡 risk: same pattern in `parseArgList`. `parseExpression()` called speculatively, then checked for `Identifier` variant. If user writes `f(a.b := 1)`, `a.b` parses as FunctionCall, `:=` not consumed → becomes positional arg `a.b` followed by parse error on `:=`. Error message won't mention "named argument syntax requires simple identifier". Add targeted diagnostic.

`parser.cpp:L160-L161`: 🔵 nit: receiver's `location` set to `method_name.location` not receiver expression's own location. For `x.foo()`, the first positional arg (the receiver `x`) gets `foo`'s location. Should use `expr->location` before it's moved. Same issue L184-L185.

## Design

`parser.cpp:L108-L146`: ❓ q: comparison is `if` not `while` — intentionally disallows chaining like `a < b < c`. This is correct for SaneQL semantics (no Python-style chained comparisons), but worth a comment explaining the deliberate choice since every other binary level uses `while`.

`parser.cpp:L115-L137`: 🔵 nit: switch-on-token-type duplicates the check already done in the `if` guard (L111-L113). Could use a helper `std::optional<BinaryOp> tryParseComparisonOp()` that returns the op and advances, or returns nullopt. Eliminates the `default: SILO_UNREACHABLE()` branch.

`parser.cpp:L260-L276`: 🔵 nit: SetLiteral parsing for non-identifier-first-element (L269-L275) is identical to the fallthrough path (L260-L266). Could merge: after the record-literal check fails, fall through to a single set-literal loop regardless of whether first element was identifier-started.

## Tests

`parser.test.cpp`: Good coverage of happy paths. Missing:
- 🟡 No test for `{a.foo() := 1}` or `{42 := 1}` — malformed record literal error paths.
- 🟡 No test for positional-after-named: `f(a:=1, 2)` should throw.
- 🟡 No test for `::` type cast chaining: `x::int::string`.
- 🟡 No test for property access without parens: `x.foo` (no `()`) — should produce `foo(x)`.
- 🔵 No test for deeply nested NOT: `!!!x`.
- 🔵 No test for empty arg list edge: `f()`.

## Style

`parser.h:L25`: 🔵 nit: `expect()` mutates state (advances token) but lacks `[[nodiscard]]` — inconsistent with other methods. Return value (the consumed token) is often used, so `[[nodiscard]]` appropriate.

`parser.h:L27`: 🔵 nit: `match()` also returns useful bool but no `[[nodiscard]]`. Less critical since callers sometimes ignore return.

`parser.cpp:L344`: 🔵 nit: `NOLINTNEXTLINE` on lambda `parse_one` is redundant — the one on L339 already covers `parseArgList`. Verify clang-tidy actually flags the lambda separately; if not, remove.

## Summary

| Severity | Count |
|----------|-------|
| 🔴 bug | 0 |
| 🟡 risk | 4 |
| 🔵 nit | 6 |
| ❓ question | 1 |

Parser is solid. Main risks: poor error messages when disambiguation fails in `parseSetOrRecordExpression` and `parseArgList`. Location tracking on desugared receiver args is slightly wrong. Test gaps around error paths and edge cases.
