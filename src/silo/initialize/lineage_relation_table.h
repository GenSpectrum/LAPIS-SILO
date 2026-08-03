#pragma once

#include <optional>
#include <string>
#include <vector>

#include "silo/common/lineage_tree.h"

namespace silo::initialize {

/// One direct parent->child edge of a lineage tree.
/// A recombinant node contributes one row per parent
struct LineageRelationRow {
   std::string lineage;
   std::optional<std::string> parent;
   bool is_recombinant_edge = false;
   std::optional<std::string> recombinant_clade_ancestor;

   bool operator==(const LineageRelationRow& other) const = default;
};

/// Builds the **direct** parent->child edges of a lineage tree: one row per canonical lineage and
/// each of its immediate parents (a recombinant yields several rows), and one root row with an
/// empty parent for each root. Aliases are not emitted as separate lineages — they resolve to their
/// canonical lineage. The transitive closure is derived from these edges at query time rather than
/// materialized here.
[[nodiscard]] std::vector<LineageRelationRow> buildLineageRelationRows(
   const common::LineageTreeAndIdMap& lineage_tree_and_id_map
);

}  // namespace silo::initialize
