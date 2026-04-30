# PR #1238 Review: `src/silo/database.cpp`

## Summary

Migration from JSON-based query interface to SaneQL. Three main changes: `getFilteredBitmap` uses SaneQL parser, `getPrevalentMutations` builds SaneQL query string via `fmt::format`, `executeQueryAsArrowIpc` simplified to single `query_string` param. Overall direction good — removes 11 action/binder includes, simplifies code paths.

---

## Findings

### `getPrevalentMutations` (L276–L312)

L286-293: 🟡 risk: **SaneQL injection via `filter` parameter.** `filter` is user-provided string interpolated raw into SaneQL query via `fmt::format`. If `filter` contains `)` or `.` it can break out of `.filter(...)` and chain arbitrary pipeline operations. Example: `filter = "true).project({primaryKey})"` → `table.filter(true).project({primaryKey}).mutations(...)`. The SaneQL parser will happily parse this as a valid pipeline. Unlike the old code path which parsed `filter` as a standalone JSON expression, here it's string-concatenated into a larger query. Fix: parse `filter` into an AST separately (like `getFilteredBitmap` does at L265-267), then pass the filter expression object directly to the planner instead of round-tripping through string concatenation. Alternatively, use `planSaneqlQuery` only for the full pipeline and construct the `MutationsNode` directly as the old code did.

L286-293: 🔵 nit: `table_name` and `sequence_name` also interpolated raw. These come from internal callers (L320-321, L331) so lower risk, but same injection class. If any caller ever passes user-controlled table/sequence names, same problem applies.

L303: 🟡 risk: `result_stream >> json_line` reads whitespace-delimited tokens, not lines. Works today because `NdjsonSink` emits compact JSON (no spaces). But if NDJSON output ever includes spaces (e.g. string values with spaces like `"mutation":"A 123 T"`), `>>` will split one JSON object across multiple reads → parse failures. Fix: use `std::getline(result_stream, json_line)` instead.

L306: 🔵 nit: **Type mismatch.** `count` field is `ColumnType::INT32` in `MutationsNode` (mutations_node.h:84), serialized via `Int32Array`. Parsed here as `uint64_t`. Works for positive values but semantically wrong. Should be `int32_t` or `uint32_t` to match Arrow schema. Pre-existing issue (old code used `SymbolMutations::COUNT_FIELD_NAME` but same `uint64_t` type), so not a regression — but worth fixing while touching this code.

L305,307: 🔵 nit: Hardcoded `"count"` and `"mutation"` strings replace `SymbolMutations::COUNT_FIELD_NAME` / `MUTATION_FIELD_NAME` constants. Loses compile-time coupling — if field names change in `MutationsNode`, these will silently break at runtime. Consider using `operators::MutationsNode<SymbolType>::COUNT_FIELD_NAME` and `MUTATION_FIELD_NAME` constants (they're still defined in mutations_node.h:21,28).

L283-284: ✅ Good: `constexpr std::string_view` with `if constexpr`-style ternary for selecting mutation function name. Clean pattern.

### `getFilteredBitmap` (L254–L274)

L265-267: ❓ q: `getFilteredBitmap` parses `filter` as standalone SaneQL expression via `Parser` + `convertToFilter`. This is the safe approach (no injection possible — parser validates syntax, `convertToFilter` only accepts filter-context AST nodes). Why doesn't `getPrevalentMutations` use the same pattern? The asymmetry is suspicious.

L260-262: 🔵 nit: Table-not-found returns empty `Roaring{}` with `SPDLOG_ERROR`. Other methods (L185, L208, L234) throw `std::runtime_error`. Inconsistent error handling. Pre-existing, not introduced by this PR.

### `executeQueryAsArrowIpc` (L480–L496)

L480: ✅ Good: Clean simplification. Single `query_string` param, delegates to `planSaneqlQuery`. No injection concern here because caller passes full query — no string interpolation.

L489-493: ✅ Good: Proper Arrow status checking with descriptive error message.

### Includes (L1–L38)

L35-36: ✅ Good: Clean swap — 11 old action/binder includes replaced by 2 saneql includes. Reduces coupling.

L30: ❓ q: `filter/expressions/true.h` still included for `printAllData` (L192). Correct, just noting it's not dead.

---

## Verdict

**Main concern:** SaneQL injection in `getPrevalentMutations` (L286-293). `filter` is user-provided and interpolated raw into query string. Should parse filter separately like `getFilteredBitmap` does, or construct operator tree directly.

**Secondary:** `>>` vs `getline` for NDJSON parsing (L303) is fragile. Low risk today, time bomb tomorrow.

Rest of changes clean and well-structured.
