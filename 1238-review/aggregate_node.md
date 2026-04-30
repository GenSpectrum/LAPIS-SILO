# Review: aggregate_node.h / aggregate_node.cpp (PR #1238)

## Overall

Good generalization from hardcoded COUNT → extensible `AggregateFunction` enum + `AggregateDefinition`. Clean Arrow Acero integration. Follows project patterns (public members, `SILO_UNREACHABLE`, `SILO_ASSERT`). Few issues below.

## Findings

### aggregate_node.h

`L22: 🟡 risk: AggregateDefinition::source_column` not validated for COUNT. COUNT ignores it, but caller can pass `source_column="bogus"` silently. When SUM/AVG added, forgetting validation here = silent wrong results. Add `SILO_ASSERT(!source_column.has_value())` in COUNT branch, or validate at construction.

`L17: 🔵 nit: enum AggregateFunction` — single-value enum fine for extensibility scaffolding, but add brief comment like `// Extended by future PRs (SUM, AVG, etc.)` so readers know it's intentional, not dead code.

### aggregate_node.cpp

`L40: 🟡 risk: source_refs` always empty. For COUNT this is correct (`count_all` takes no source), but when SUM/MIN/MAX added, `source_refs` must be populated from `agg.source_column`. Current structure doesn't make this obvious — the empty vector is constructed then moved without any branch populating it. Consider adding a comment or an assert: `SILO_ASSERT(source_refs.empty())` in COUNT branch to make the invariant explicit.

`L32: 🔵 nit: input_schema` param only used in L67 assert. In release builds with asserts compiled out, this becomes an unused parameter. Either `[[maybe_unused]]` or restructure so the schema validation is always active (return error instead of assert).

`L67: 🟡 risk: SILO_ASSERT for schema validation.` `CanReferenceFieldByName` check is debug-only. If group_by field doesn't exist in input schema, release build silently passes bad field ref to Arrow → runtime crash in Arrow internals with unhelpful error. Should be a proper error return (`arrow::Status::Invalid(...)`) not an assert.

`L90: 🔵 nit: uninitialized local` `schema::ColumnType type;` — technically fine because switch covers all enum values and compiler warns on missing cases, but initializing to a sentinel or using a helper function (like `arrowFunctionName` pattern) would be more defensive. If someone adds enum value and forgets this switch, UB from uninitialized read.

`L4-6: 🔵 nit: unused includes.` `<map>`, `<string>`, `<vector>` — these are already transitively included via the header. Not wrong, but the header already includes them. Project style seems to prefer explicit includes so this is fine, just noting.

### Missing

`❓ q: No unit tests for AggregateNode.` `aggregate_node.test.cpp` doesn't exist. The generalization from hardcoded COUNT to configurable aggregates is a behavioral change — should have at least: (1) test COUNT with no groups, (2) test COUNT with groups, (3) test empty aggregates vector, (4) test `getOutputSchema` returns correct types. Integration coverage via e2e tests may exist but unit tests catch regressions faster.

## Summary

| Severity | Count |
|----------|-------|
| 🔴 Critical | 0 |
| 🟡 Risk | 3 |
| 🔵 Nit | 3 |
| ❓ Question | 1 |

Main concern: L67 assert-only validation of group_by fields against input schema. Release builds skip this → bad field refs hit Arrow internals. Convert to proper error return.
