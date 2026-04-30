# PR #1238 — Unresolved Node Headers Review

Files reviewed:
- `unresolved_phylo_subtree_node.h`
- `unresolved_most_recent_common_ancestor_node.h`
- `unresolved_mutations_node.h`
- `unresolved_insertions_node.h`

---

## Findings

### 🟡 risk: All 4 files — Missing `#include <stdexcept>`

All four files throw `std::runtime_error` in `toQueryPlan()` but none include `<stdexcept>`.
Currently compiles only because some transitive include chain (likely through Arrow or `<map>`)
pulls it in. This is fragile — any include reordering or library upgrade can break compilation.

- `unresolved_phylo_subtree_node.h:36`
- `unresolved_most_recent_common_ancestor_node.h:33`
- `unresolved_mutations_node.h:38`
- `unresolved_insertions_node.h:29`

**Fix:** Add `#include <stdexcept>` to each file.

### 🟡 risk: All 4 files — `getOutputSchema()` returns empty `{}`

`getOutputSchema()` returns empty vector in all unresolved nodes. This is called during
AST construction by `handleGroupBy` (ast_to_query.cpp:610) and `handleProject`
(ast_to_query.cpp:645) on child nodes. If a user composes e.g.
`project(mutations(...), fields: ...)` or `groupBy(mutations(...), ...)`, the empty schema
causes `CHECK_SILO_QUERY` to fail with a confusing "field X not present in output schema"
error instead of a clear "this node must be resolved first" message.

Also called in `wrapWithDecompressIfNeeded` (planner.cpp:51) — empty `{}` silently skips
decompression wrapping, which is benign since pushdown replaces the node, but still a
latent correctness concern if call order ever changes.

**Two options:**
1. **Throw** like `toQueryPlan()` does — makes the contract explicit: "don't call this before pushdown."
2. **Document** that empty `{}` is intentional and these nodes must always be leaf/terminal during AST construction (currently true for mutations/insertions/phylo/mrca, but not enforced).

Option 1 preferred — fail loud, fail early.

### 🔵 nit: All 4 files — `std::runtime_error` vs project exception types

Project has `QueryCompilationException` and `IllegalQueryException` (both derive
`std::runtime_error`). An unresolved node surviving to execution is an internal logic error,
not a user query error. Consider using `SILO_ASSERT` or a dedicated internal error type
instead of bare `std::runtime_error`. This would make it easier to distinguish "bug in
planner" from "bad user query" in error handling.

### 🔵 nit: All 4 files — Public member variables

All members are public. Consistent with resolved counterparts (`MutationsNode`,
`InsertionsNode`, `PhyloSubtreeNode`, `MostRecentCommonAncestorNode` all have public
members too), so this follows existing project convention. No action needed — just noting
for awareness.

### 🔵 nit: Template vs non-template inconsistency

`UnresolvedMutationsNode<SymbolType>` and `UnresolvedInsertionsNode<SymbolType>` are
templates, while `UnresolvedPhyloSubtreeNode` and `UnresolvedMostRecentCommonAncestorNode`
are not. This mirrors the resolved counterparts (`MutationsNode<T>`, `InsertionsNode<T>`
are templates; `PhyloSubtreeNode`, `MostRecentCommonAncestorNode` are not). Consistent
with existing pattern. `SymbolType` is used purely as a type tag for `dynamic_cast`
dispatch in planner.cpp:388-401 — valid pattern, no issue.

---

## Summary

| Severity | Count | Summary |
|----------|-------|---------|
| 🟡 risk  | 2     | Missing `<stdexcept>`, empty `getOutputSchema()` |
| 🔵 nit   | 2     | Exception type choice, public members (both follow convention) |

**Overall:** Clean, minimal placeholder nodes. Two real risks: missing include (fragile build)
and silent empty schema (confusing error on composition). Both straightforward fixes.
No bugs that would cause runtime crashes in normal usage paths.
