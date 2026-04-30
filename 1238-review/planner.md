# Review: `planner.cpp` / `planner.h` (PR #1238)

Overall: solid design. Two-phase planner (pushdown → optimize) clean and readable. Findings below.

---

## Bugs / Risks

`planner.cpp:L286-287`: 🟡 risk: double `dynamic_cast<TableScanNode*>` — first in condition check (L286), second to get pointer (L287). Wasteful + fragile if condition changes. Assign once.
```cpp
auto* table_scan_child = dynamic_cast<operators::TableScanNode*>(node->child.get());
if (node->group_by_fields.empty() && node->aggregates.size() == 1 &&
    node->aggregates[0].function == operators::AggregateFunction::COUNT &&
    table_scan_child != nullptr) {
```

`planner.cpp:L438-449`: 🟡 risk: same double-cast pattern in `optimize()`. Each branch does `dynamic_cast` to check null, then immediately casts again. Combine into single `if (auto* x = dynamic_cast<...>(...))`.
```cpp
if (auto* aggregate = dynamic_cast<operators::AggregateNode*>(node.get())) {
   return optimizeInstance(aggregate);
}
// same for OrderByNode, FetchNode
```

`planner.cpp:L361-364`: 🟡 risk: dangling pointer comparison. `before` points into old `node`. After `tryReorderProject(std::move(node))`, `node` is reassigned. Comparing `node.get() != before` works only because `tryReorderProject` returns original `node` unchanged (same unique_ptr) when no reorder happens. If `tryReorderProject` ever wraps/copies the node without reordering, this breaks silently. Fragile contract. Consider returning a `bool` or `std::optional` from `tryReorderProject` instead.

`planner.cpp:L79-96`: ❓ q: `extractScanInfo` only handles `ScanNode` and `FilterNode(ScanNode)` — not `FilterNode(FilterNode(ScanNode))` or deeper chains. Is this guaranteed by AST construction? If SaneQL parser can produce nested filters, mutations/insertions pushdown silently fails with "must be applied to a table scan". Worth a comment documenting this invariant.

`planner.cpp:L370-383`: 🟡 risk: `pushdown` handles `FilterNode(ScanNode)` and `FilterNode(ProjectNode)` for collapse, but not `FilterNode(FilterNode(ScanNode))`. If two WHERE clauses get stacked, neither collapses into `TableScanNode`. `pushdownScanFilterProject` loop (L239-253) only peels one of each type. Multiple filters → falls through, returns original node → `FilterNode.toQueryPlan()` throws at runtime. Either merge stacked filters during pushdown or document this as impossible from parser.

---

## Major

`planner.cpp:L437-451`: 🟡 `optimize()` doesn't recurse into `ProjectNode`, `FilterNode`, `ZstdDecompressNode`, `TableScanNode`, or any unresolved nodes. After pushdown, most should be gone, but `ZstdDecompressNode` wraps `TableScanNode` (L59) and `optimize()` won't look inside it. If `Aggregate(ZstdDecompress(TableScan))` appears, COUNT(*) optimization misses. Probably fine today (COUNT doesn't need decompression), but fragile — add a comment or a catch-all recurse.

`planner.cpp:L462-468`: 🟡 `planQuery` catches Arrow errors via `result.ok()` but throws `std::runtime_error`. `planSaneqlQuery` (L470-478) doesn't catch `IllegalQueryException` from pushdown. Caller gets two different exception types for query errors. Inconsistent error contract. Consider unifying or documenting.

---

## Minor

`planner.cpp:L265`: 🔵 nit: `std::unordered_set<std::string> seen_names` — dedup by name only. If two `ColumnIdentifier`s have same name but different types (shouldn't happen but defensive), first one wins silently. Fine if schema guarantees uniqueness.

`planner.cpp:L126`: 🔵 nit: `std::vector<std::string_view> fields_to_use` stores views into static data (L129-137) and into `unresolved->fields` (L147). If `unresolved` is moved/destroyed before `fields_to_use` consumed, dangling views. Currently safe because `fields_to_use` consumed immediately at L151, but fragile if refactored.

`planner.h:L14`: 🔵 nit: `std::map` and `std::shared_ptr` used in header without `#include <map>` and `#include <memory>`. Compiles because `query_node.h` transitively includes them, but violates include-what-you-use.

`planner.h:L10-31`: 🔵 nit: `Planner` is all static methods, no state. Could be a namespace with free functions instead of a class. Class is fine if you plan to add state later (e.g., optimizer config).

`planner.cpp:L467`: 🔵 nit: `std::move(result.ValueUnsafe())` — `ValueUnsafe()` skips status check. Already checked `result.ok()` above so safe, but `result.MoveValueUnsafe()` is more idiomatic Arrow for this pattern.

---

## Design

`planner.cpp:L239-253`: The 3-iteration loop for peeling Project/Filter/Scan is clever but non-obvious. Reader must reason about all 6 permutations. The doc comment (L226-228) helps. Consider: if a 4th node type ever needs collapsing, loop bound `3` becomes wrong silently. A `while(true)` with explicit break conditions would be more robust.

`planner.cpp:L355-433`: `pushdown()` is a long if-else chain of `dynamic_cast`. Classic visitor pattern candidate. Not blocking — current approach works and is explicit. But if node types keep growing, consider `std::variant` or virtual `accept()`.

---

## Good

- `pushdownScanFilterProject` loop handles all Scan/Filter/Project orderings cleanly
- `tryReorderProject` safety check for sort keys is correct and well-documented
- `wrapWithDecompressIfNeeded` is clean separation of concerns
- `CHECK_SILO_QUERY` gives good error messages with context
- Template pushdown for `Nucleotide`/`AminoAcid` avoids duplication
