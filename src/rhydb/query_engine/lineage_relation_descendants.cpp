#include "rhydb/query_engine/lineage_relation_descendants.h"

#include <deque>
#include <unordered_map>
#include <vector>

#include "rhydb/storage/column/bool_column.h"
#include "rhydb/storage/column/row_id.h"
#include "rhydb/storage/column/string_column.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine {

using common::RecombinantEdgeFollowingMode;

namespace {

// Column names of a lineage relation table (see Initializer::createLineageRelationTable).
constexpr std::string_view LINEAGE_COLUMN = "lineage";
constexpr std::string_view PARENT_COLUMN = "parent";
constexpr std::string_view IS_RECOMBINANT_EDGE_COLUMN = "is_recombinant_edge";
constexpr std::string_view RECOMBINANT_CLADE_ANCESTOR_COLUMN = "recombinant_clade_ancestor";
// Column name of a lineage alias table (see Initializer::createLineageAliasTable); its second
// column is LINEAGE_COLUMN.
constexpr std::string_view ALIAS_COLUMN = "alias";

}  // namespace

std::string resolveLineageAlias(const storage::Table& alias_table, const std::string& name) {
   const auto& alias_column = alias_table.columns.string_columns.at(std::string{ALIAS_COLUMN});
   const auto& lineage_column = alias_table.columns.string_columns.at(std::string{LINEAGE_COLUMN});
   for (const storage::column::RowId row : alias_table.row_layout) {
      if (alias_column.getValueString(row) == name) {
         return lineage_column.getValueString(row);
      }
   }
   return name;
}

std::optional<std::unordered_set<std::string>> computeLineageWithSublineages(
   const storage::Table& relation_table,
   const std::string& target,
   RecombinantEdgeFollowingMode mode
) {
   const auto& columns = relation_table.columns;
   const auto& lineage_column = columns.string_columns.at(std::string{LINEAGE_COLUMN});
   const auto& parent_column = columns.string_columns.at(std::string{PARENT_COLUMN});
   const auto& clade_ancestor_column =
      columns.string_columns.at(std::string{RECOMBINANT_CLADE_ANCESTOR_COLUMN});
   const auto& is_recombinant_edge_column =
      columns.bool_columns.at(std::string{IS_RECOMBINANT_EDGE_COLUMN});

   // parent -> children under the requested recombinant-following mode. Rows with a null parent
   // (the roots) contribute no edge, but every lineage is still recorded so `target` can be
   // validated and matched even when it is isolated.
   std::unordered_map<std::string, std::vector<std::string>> children;
   bool target_is_a_lineage = false;

   for (const storage::column::RowId row : relation_table.row_layout) {
      std::string lineage = lineage_column.getValueString(row);
      if (lineage == target) {
         target_is_a_lineage = true;
      }
      if (parent_column.isNull(row)) {
         continue;
      }
      if (!is_recombinant_edge_column.getValue(row)) {
         children[parent_column.getValueString(row)].push_back(std::move(lineage));
         continue;
      }
      switch (mode) {
         case RecombinantEdgeFollowingMode::ALWAYS_FOLLOW:
            children[parent_column.getValueString(row)].push_back(std::move(lineage));
            break;
         case RecombinantEdgeFollowingMode::DO_NOT_FOLLOW:
            break;
         case RecombinantEdgeFollowingMode::FOLLOW_IF_FULLY_CONTAINED_IN_CLADE:
            if (!clade_ancestor_column.isNull(row)) {
               children[clade_ancestor_column.getValueString(row)].push_back(std::move(lineage));
            }
            break;
      }
   }

   if (!target_is_a_lineage) {
      return std::nullopt;
   }

   std::unordered_set<std::string> result;
   std::deque<std::string> queue;
   result.insert(target);
   queue.push_back(target);
   while (!queue.empty()) {
      const std::string current = std::move(queue.front());
      queue.pop_front();
      const auto found = children.find(current);
      if (found == children.end()) {
         continue;
      }
      for (const auto& child : found->second) {
         if (result.insert(child).second) {
            queue.push_back(child);
         }
      }
   }
   return result;
}

}  // namespace rhydb::query_engine
