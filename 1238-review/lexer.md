# Lexer Review — PR #1238

**Files:** `lexer.cpp` (321L), `lexer.h` (38L), `lexer.test.cpp` (264L)

## Summary

Solid lexer. Clean structure, good error messages, proper `unsigned char` casts for `isdigit`/`isalpha`. Tests cover happy paths well. Few issues below.

---

## Findings

### lexer.cpp

`L178-179`: 🔵 nit: `peek() == '.'` in while-condition is dead — L181-183 immediately `break`s on dot. Remove `|| peek() == '.'` from condition, it's misleading. Reader thinks dots get consumed into identifiers.

`L34`: 🟡 risk: `advance()` has no guard against `isAtEnd()`. If called when `position >= input.size()`, `input[position]` is UB. Currently all callers check before calling, but one missed check = crash. Add `SILO_ASSERT(!isAtEnd())` or a bounds check.

`L226-228`: 🔴 bug: `-42` after any token becomes negative number, not minus + number. Input `x -42` → `[IDENTIFIER("x"), INT_LITERAL(-42)]` instead of `[IDENTIFIER("x"), ???, INT_LITERAL(42)]`. No MINUS token exists, so parser can't distinguish `a - b` from `a` followed by `-b`. This is fine **only if** subtraction is not in the grammar. If subtraction ever gets added, this will silently misparse. Add a comment documenting this design decision, or add a MINUS token now.

`L147`: 🔵 nit: `1.` (trailing dot, no digit after) parses as `INT_LITERAL(1)` then `DOT`. This is correct for method-call syntax (`1.toString()`), but worth a test to lock in the behavior.

`L305`: 🔵 nit: `fmt::format` used but `<fmt/format.h>` not directly included — relies on transitive include through `parse_exception.h`. Add explicit include per project style (include what you use).

`L173`: 🟡 risk: `throwsOnUnterminatedString` test creates unused `const Lexer lexer` at L174 before the lambda. Dead code in test — harmless but confusing.

`L93-95`: ❓ q: Unknown escape sequences like `\x` produce literal `\x` (backslash preserved). Is this intentional? SQL standard doesn't have C-style escapes at all. Consider throwing on unknown escapes to catch typos early, or document the pass-through behavior.

`L50-53`: 🔵 nit: Line comment (`--`) consumed inside `skipWhitespace`. Works, but comment at end of input (no trailing newline) silently works because `isAtEnd()` terminates the inner while. Good — just noting it's correct.

### lexer.h

`L14`: 🔵 nit: `SourceLocation current_location;` — default-initialized via aggregate default `{1,1}`. Correct but could add `{}` or `{1,1}` for explicitness since `position` has `= 0`.

### lexer.test.cpp — Missing test coverage

- No test for negative numbers (`-42`, `-3.14`)
- No test for `1.` (trailing dot) behavior
- No test for bare `-` (should it throw? currently throws "Unexpected character")
- No test for empty quoted identifier `""` (produces empty string IDENTIFIER — intentional?)
- No test for `0` or `INT64_MIN`/`INT64_MAX` overflow
- No test for float overflow/underflow
- No test for comment at end of input with no trailing newline (`"a -- comment"`)
- No test for `\r\n` line endings (column tracking)
- No test for `!=` (should throw — documents that `<>` is the only not-equals)

---

## Good stuff

- `static_cast<unsigned char>` on all `isdigit`/`isalpha` calls — prevents UB with signed char. 👍
- `readQuotedIdentifier` correctly handles SQL `""` escaping convention. 👍
- Error messages include source location — great for user-facing errors. 👍
- `[[nodiscard]]` on all query methods in header. 👍
- Test for full method-call chain (`tokenizesMethodCallChain`) is thorough. 👍
