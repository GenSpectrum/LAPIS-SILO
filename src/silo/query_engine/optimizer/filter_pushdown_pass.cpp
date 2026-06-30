#include "silo/query_engine/optimizer/filter_pushdown_pass.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/expressions/and.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/phylo_subtree_node.h"
#include "silo/query_engine/operators/table_scan_node.h"
#include "silo/query_engine/operators/union_all_node.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::optimizer {

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::FilterNode& node) {
   current_filters.push_back(std::move(node.filter));
   auto child = std::move(node.child);
   propagateToNode(child);
   return child;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::MapNode& node) {
   // A MapNode produces (or replaces) columns via scalar assignments. A filter whose
   // predicate consumes a produced column MUST NOT be pushed below this Map: below the Map
   // that produced value does not exist yet. The motivating case is the decompression Map,
   // which produces a STRING `seq` from a sequence-typed `seq` of the same name. A filter
   // on the decompressed STRING (e.g. StringEquals) must stay above the Map; a filter that
   // consumes the underlying sequence column directly (e.g. SymbolEquals / HasMutation,
   // which compile to bitmap operators on the sequence-typed storage) must still push down
   // to the scan.
   //
   // We distinguish the two by the type the predicate reports for the column in freeIUs:
   // sequence predicates report the sequence type (they reference the Map's input column,
   // which survives below the Map), while predicates that operate on the produced STRING
   // report a non-sequence placeholder type. So a filter is dependent (stays above) only
   // when it references a produced column name with a non-sequence type.
   const auto references_produced_column =
      [&](const std::unique_ptr<expressions::Expression>& filter) {
         const auto referenced = filter->freeIUs();
         return std::ranges::any_of(node.assignments, [&](const auto& assignment) {
            return std::ranges::any_of(referenced, [&](const auto& column) {
               return column.name == assignment.output_column.name &&
                      !schema::isSequenceColumn(column.type);
            });
         });
      };

   std::vector<std::unique_ptr<expressions::Expression>> dependent_filters;
   std::vector<std::unique_ptr<expressions::Expression>> independent_filters;
   for (auto& filter : current_filters) {
      if (references_produced_column(filter)) {
         dependent_filters.push_back(std::move(filter));
      } else {
         independent_filters.push_back(std::move(filter));
      }
   }

   // Only the independent filters keep flowing down past the Map.
   current_filters = std::move(independent_filters);
   propagateToNode(node.child);

   if (dependent_filters.empty()) {
      return nullptr;
   }

   // Re-erect the filters that depend on produced columns as a single FilterNode directly
   // above the Map, AND-merging when there is more than one.
   std::unique_ptr<expressions::Expression> surviving_filter =
      std::make_unique<expressions::And>(std::move(dependent_filters));

   // Returning a replacement node swaps out the visited MapNode. Move the visited Map's
   // contents into a fresh MapNode and return Filter(Map(child)), keeping the dependent
   // filters above the Map.
   auto map =
      std::make_unique<operators::MapNode>(std::move(node.child), std::move(node.assignments));
   return std::make_unique<operators::FilterNode>(std::move(map), std::move(surviving_filter));
}

operators::QueryNodePtr FilterPushdownPass::operator()(operators::TableScanNode& node) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}

operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::MutationsNode<silo::Nucleotide>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}

operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::MutationsNode<silo::AminoAcid>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::InsertionsNode<silo::Nucleotide>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(
   operators::InsertionsNode<silo::AminoAcid>& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(operators::PhyloSubtreeNode& node) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}
operators::QueryNodePtr FilterPushdownPass::operator()(operators::MostRecentCommonAncestorNode& node
) {
   current_filters.push_back(std::move(node.filter));
   node.filter = std::make_unique<expressions::And>(std::move(current_filters));
   return nullptr;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr FilterPushdownPass::operator()(operators::UnionAllNode& node) {
   // Push parent filters into both children. Clone for right, move originals into left.
   FilterPushdownPass right_pass;
   for (const auto& filter : current_filters) {
      right_pass.current_filters.push_back(filter->clone());
   }
   FilterPushdownPass left_pass;
   left_pass.current_filters = std::move(current_filters);

   left_pass.propagateToNode(node.left);
   right_pass.propagateToNode(node.right);
   return nullptr;
}

}  // namespace silo::query_engine::optimizer
