# PR #1238 — ast.h / ast.cpp Review

## Summary

Clean AST design. Variant-based node types, consistent extract/check helpers, good error messages with source locations. Few real issues, mostly precision/consistency concerns.

---

## Findings

### ast.cpp

`L143-153: 🟡 risk: extractFloatLiteral silently converts int64_t→double. int64_t values >2^53 lose precision. Callers (minProportion etc.) unlikely to hit this, but no guard exists. Add range check or document assumption.`

`L241-243 vs L143-153: 🟡 risk: isFloatLiteral() returns false for IntLiteral, but extractFloatLiteral() accepts IntLiteral. Semantic mismatch — caller doing if(isFloatLiteral(x)) extractFloatLiteral(x) works, but if(!isFloatLiteral(x)) doesn't mean extractFloatLiteral will throw. Consider isNumericLiteral() or rename to extractNumericAsFloat().`

`L9-28: 🔵 nit: binaryOpToString — L28 return "?" after switch covering all enum values. Compiler warns on missing enum case already. Replace with std::unreachable() (C++23) or SILO_ASSERT(false) to catch corruption instead of silently returning "?".`

`L143-153: 🔵 nit: extractFloatLiteral uses explicit throw while all other extract* fns use CHECK_SILO_QUERY macro. Inconsistent error-handling style. Use CHECK_SILO_QUERY with a combined holds_alternative check, or document why this one is different.`

`L56-71: 🔵 nit: FunctionCall::toString builds args string via repeated += concatenation. Fine for small arg lists. Consider fmt::join or std::ostringstream if arg counts grow. Not blocking.`

### ast.h

`L99-111: ❓ q: ExpressionVariant has 12 types. std::variant visit generates jump table — fine for correctness. Any profiling data on variant dispatch overhead in hot query paths? If toString() is debug-only, no concern. If extract* called per-row, might matter.`

`L122-130: 🔵 nit: extract* functions return by value (string, vector). Fine for move semantics. extractSetLiteral returns const ref — good. Consider returning std::string_view from extractIdentifierName/extractStringLiteral if callers don't need ownership (avoids copy).`

### ast_to_query.cpp (related — not in review scope but worth noting)

`ast_to_query.cpp:L74,184,284,287,376,401,411,737: 🟡 risk: static_cast<uint32_t>(extractIntLiteral(...)) — int64_t→uint32_t truncation. Negative values or values >UINT32_MAX silently wrap. Should validate range before cast. This is in the caller, not ast.cpp, but the pattern is pervasive and the AST could provide a safe extractUint32Literal().`

### Testing

`🟡 risk: No test file found for ast.h/ast.cpp (no ast.test.cpp). Extract functions have non-trivial logic (type coercion, date validation, set extraction). Unit tests needed — especially for extractFloatLiteral int→double edge cases, extractDateValue with invalid dates, and the isX/extractX semantic contract.`

---

## Good Practices

- SourceLocation in every error message — excellent for user-facing diagnostics
- `[[nodiscard]]` on all query functions — prevents silent discard bugs
- `ExpressionPtr` = unique_ptr — clear ownership, no leaks
- extractDateValue validates via stringToDate32 and propagates error string — thorough
- extractOptionalDateValue cleanly composes with NullLiteral check — nice pattern
- Variant-based AST avoids inheritance hierarchy — good modern C++ choice

---

## Verdict

Solid code. Main actionable items:
1. **isFloatLiteral/extractFloatLiteral semantic mismatch** — rename or add isNumericLiteral
2. **int64_t→double precision loss** — add guard in extractFloatLiteral
3. **No unit tests** — add ast.test.cpp
4. **binaryOpToString unreachable** — use std::unreachable() instead of return "?"
