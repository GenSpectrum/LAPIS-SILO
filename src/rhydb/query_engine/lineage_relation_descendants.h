#pragma once

#include <optional>
#include <string>
#include <unordered_set>

#include "rhydb/common/lineage_tree.h"

namespace rhydb::storage {
class Table;
}

namespace rhydb::query_engine {

/// Maps `name` through a lineage alias table (as built by Initializer::createLineageAliasTable:
/// columns `alias`, `lineage`) to the canonical lineage it stands for. Returns `name` unchanged
/// when it is not an alias, so a canonical lineage passes through untouched.
[[nodiscard]] std::string resolveLineageAlias(
   const storage::Table& alias_table,
   const std::string& name
);

/// Computes `target` together with all of its sublineages by traversing the parent->child edges of
/// a lineage relation table (as built by Initializer::createLineageRelationTable: columns
/// `lineage`, `parent`, `is_recombinant_edge`, `recombinant_clade_ancestor`). The
/// recombinant-following `mode` selects which recombinant edges are traversed, mirroring
/// `LineageTree::getAllParents`:
///   - ALWAYS_FOLLOW: follow every edge.
///   - DO_NOT_FOLLOW: do not descend into recombinant nodes.
///   - FOLLOW_IF_FULLY_CONTAINED_IN_CLADE: reach a recombinant node only from its clade ancestor.
///
/// Returns the matching lineage names (which equal the data column's dictionary values, so the
/// caller can turn them into a bitmap union), or `std::nullopt` if `target` is not a lineage of the
/// relation table. The result always contains `target` itself (getAllParents is reflexive), even
/// for an isolated lineage that has no edges.
[[nodiscard]] std::optional<std::unordered_set<std::string>> computeLineageWithSublineages(
   const storage::Table& relation_table,
   const std::string& target,
   common::RecombinantEdgeFollowingMode mode
);

}  // namespace rhydb::query_engine
