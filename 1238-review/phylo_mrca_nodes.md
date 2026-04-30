# PR #1238 Review: phylo_subtree_node / most_recent_common_ancestor_node

## Critical

`phylo_subtree_node.cpp:L103-109` / `most_recent_common_ancestor_node.cpp:L101-107`: 🔴 bug: `resolved_table` null-deref if constructed with neither `table` nor `table_name`. Both constructors guarantee one-or-the-other, but nothing prevents default-constructing `table` (nullptr) + `table_name` (nullopt) via aggregate init or future refactor. The `if (!resolved_table && table_name.has_value())` guard silently falls through → `*resolved_table` on L109/L107 = UB. Add `SILO_ASSERT(resolved_table)` or `CHECK_SILO_QUERY(resolved_table, "no table resolved")` after the if-block.

## Major

`phylo_subtree_node.cpp:L26-56` / `most_recent_common_ancestor_node.cpp:L26-56`: 🟡 risk: `NodeValuesResult` struct + `getNodeValuesFromTable()` are **identical** copy-paste across both files (same anonymous namespace, same 30 lines). Extract to shared header/cpp (e.g., `compute_filter.h` or new `node_values_util.h`). Duplication = divergence risk when one gets fixed and other doesn't.

## Minor

`phylo_subtree_node.h:L7` / `most_recent_common_ancestor_node.h:L7`: 🔵 nit: `#include <string_view>` unused in both headers. Neither file uses `std::string_view`. Remove.

`phylo_subtree_node.h:L22-27` / `most_recent_common_ancestor_node.h:L22-26`: 🔵 nit: All members public. `table`, `filter`, `column_name`, `table_name` are construction-time invariants never mutated after ctor. Consider making them private or at least documenting why public (serialization? test access?).

`phylo_subtree_node.cpp:L143` / `most_recent_common_ancestor_node.cpp:L142`: 🟡 risk: `&phylo_tree` captured by reference in lambda. Safe only because `resolved_table` (shared_ptr, captured by value as `table_handle`) keeps the schema alive. Correct but fragile — if someone removes `table_handle` capture, `phylo_tree` dangles. Add comment explaining lifetime dependency.

## Questions

`phylo_subtree_node.cpp:L75-86` / `most_recent_common_ancestor_node.cpp:L73-82`: ❓ q: `TableName` constructor exists but is never called anywhere in codebase (only `shared_ptr<Table>` ctor used from planner.cpp:L200/L218). Dead code? If planned for future use, add a test exercising this path. If not, remove to reduce surface area.

## Summary

- 1 critical null-deref risk in table resolution
- 30 lines of exact duplication across files
- TableName ctor path untested/unused
- Minor: unused include, public members, fragile ref capture
