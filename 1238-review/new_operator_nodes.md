# PR #1238 — New Operator Nodes Review

## Summary

Three new logical operator nodes: `FilterNode`, `ProjectNode`, `ScanNode`. Intermediate representations between AST conversion and pushdown. Collapsed into `TableScanNode` during pushdown via `pushdownScanFilterProject()`.

Overall: clean, well-structured, follows existing patterns. Few issues below.

---

## Findings

### ProjectNode

`project_node.h:L16`: 🟡 risk: Comment says "Must be eliminated during pushdown" but `ProjectNode` has a real `toQueryPlan()` impl and CAN survive pushdown (planner.cpp:L424-426 recurses into it without collapsing when child isn't Scan/Filter). Comment is misleading. Either remove "Must be eliminated" or say "Collapsed into TableScanNode when possible; otherwise executes as Arrow project."

`project_node.cpp:L27-29`: 🔵 nit: `field.name` used for both expression and output name. Correct for simple column refs, but if `ColumnIdentifier` ever gains qualified names (table.column), `field_ref(field.name)` may break. Fine for now, just noting.

`project_node.cpp:L17-38`: ❓ q: No guard for empty `fields`. Arrow `ProjectNodeOptions` with zero expressions — is that valid? If `fields` is empty, this creates a zero-column projection. Probably unreachable from parser, but a defensive check or assert would be cheap insurance.

### FilterNode

`filter_node.h:L17`: 🟡 risk: Same "Must be eliminated during pushdown" claim, but planner.cpp:L428-431 recurses into `FilterNode` without collapsing when child isn't Scan/Project. If a `FilterNode` survives with e.g. an `AggregateNode` child, `toQueryPlan()` throws `std::runtime_error` at query time. Currently unreachable from parser (filter always wraps scan), but the planner structure allows it. Two options: (a) implement a real `toQueryPlan` like `ProjectNode` does (delegate to child, apply Arrow filter), or (b) add a `SILO_ASSERT` in pushdown that no FilterNode/ScanNode survives, so the invariant is enforced rather than silently assumed.

`filter_node.h:L20-21`: 🔵 nit: Public data members. Consistent with other nodes (`UnresolvedMutationsNode`, `TableScanNode`), so follows project convention. Just noting — if these were new patterns, would suggest accessor methods.

`filter_node.cpp:L19`: 🔵 nit: Throws `std::runtime_error`. Other "must be eliminated" nodes (`UnresolvedMutationsNode`) also throw `std::runtime_error`. Consistent. But `ScanNode` uses `fmt::format` in its throw while `FilterNode` uses a plain string literal — minor inconsistency. Consider adding the node type or filter info to the error message for debuggability, like ScanNode does with table name.

### ScanNode

`scan_node.h:L5`: 🔵 nit: `#include <string>` — `string` not directly used in header. `schema::TableName` and `schema::ColumnIdentifier` come from `database_schema.h`. Remove unless needed for transitive reasons.

`scan_node.cpp:L24-27`: ✅ Good: includes table name in error message via `fmt::format`. Better debuggability than FilterNode's plain string.

`scan_node.h:L21`: ❓ q: `output_schema` stores full column list at construction. This is a snapshot of the table schema at AST-conversion time. If table schema could change between parse and execute (hot reload?), this could go stale. Probably not an issue in current architecture, but worth confirming.

### Cross-cutting

**No tests**: No `*.test.cpp` for any of these three nodes. `FilterNode` and `ScanNode` throw in `toQueryPlan` so unit-testing them directly is limited, but `ProjectNode` has real logic worth testing. At minimum:
- `ProjectNode::toQueryPlan` with a mock/stub child
- `getOutputSchema()` returns correct fields for all three
- `FilterNode::toQueryPlan` throws as expected

**Memory ownership**: ✅ Good. `unique_ptr` for child nodes and filter expressions, moved in constructors. No raw owning pointers. Consistent with codebase.

**Include completeness**: `filter_node.h` includes `<map>`, `<memory>`, `<vector>`, `arrow/result.h`, expression.h, query_node.h, database_schema.h, table.h. The `<map>` and `table.h` are needed for `toQueryPlan` signature inherited from base. `expression.h` needed for `filter` member. All correct. Same analysis holds for other headers.

**Style**: ✅ 3-space indent, `#pragma once`, Chromium braces, `camelBack` methods, `snake_case` members. All correct per AGENTS.md.

---

## Verdict

Solid implementation. Main concern: `FilterNode` comment/behavior mismatch with planner (latent crash path). `ProjectNode` comment inaccurate. Missing tests for `ProjectNode::toQueryPlan` logic.

| Severity | Count |
|----------|-------|
| 🔴 Critical | 0 |
| 🟡 Risk | 2 |
| 🔵 Nit | 4 |
| ❓ Question | 2 |
