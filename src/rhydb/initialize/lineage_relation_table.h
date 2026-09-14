#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "rhydb/common/lineage_tree.h"

namespace rhydb::initialize {

/// The name of the table holding the alias -> lineage pairs of the lineage definition
/// `definition_name`. No such table exists when the definition declares no aliases.
[[nodiscard]] std::string lineageAliasTableName(std::string_view definition_name);

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

/// One alias of a lineage tree, and the canonical lineage it stands for.
struct LineageAliasRow {
   std::string alias;
   std::string lineage;

   bool operator==(const LineageAliasRow& other) const = default;
};

/// Builds the alias -> canonical lineage pairs of a lineage tree. They live beside the edges
/// rather than among them, so that the edge table keeps holding canonical lineages only: an alias
/// is another name for a lineage, not a lineage of its own. Empty when the definition declares no
/// aliases.
[[nodiscard]] std::vector<LineageAliasRow> buildLineageAliasRows(
   const common::LineageTreeAndIdMap& lineage_tree_and_id_map
);

}  // namespace rhydb::initialize
