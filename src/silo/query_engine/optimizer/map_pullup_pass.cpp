#include "silo/query_engine/optimizer/map_pullup_pass.h"

#include <memory>
#include <string>
#include <unordered_set>

#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/map_node.h"
#include "silo/query_engine/operators/order_by_node.h"

namespace silo::query_engine::optimizer {

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr MapPullupPass::operator()(operators::FetchNode& node) {
   propagateToNode(node.child);

   // A FetchNode only limits/offsets rows; it references no columns, so a MapNode below
   // it can always be pulled up.
   if (node.child->kind() != operators::NodeKind::MAP) {
      return nullptr;
   }
   operators::QueryNodePtr map_owner = std::move(node.child);
   auto& map = static_cast<operators::MapNode&>(*map_owner);

   auto new_fetch =
      std::make_unique<operators::FetchNode>(std::move(map.child), node.count, node.offset);
   map.child = std::move(new_fetch);
   return map_owner;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr MapPullupPass::operator()(operators::OrderByNode& node) {
   propagateToNode(node.child);

   if (node.child->kind() != operators::NodeKind::MAP) {
      return nullptr;
   }
   auto& map = static_cast<operators::MapNode&>(*node.child);

   // The columns the MapNode produces (newly added or replaced in place).
   std::unordered_set<std::string> produced_columns;
   for (const auto& assignment : map.assignments) {
      produced_columns.insert(assignment.output_column.name);
   }

   // Pulling the MapNode above the OrderBy is only safe when no order-by field references a
   // produced column. Otherwise the OrderBy below would sort on a different value (a replaced
   // column) or a column that does not exist yet (a newly added one), changing the result.
   for (const auto& field : node.fields) {
      if (produced_columns.contains(field.field.name)) {
         return nullptr;
      }
   }

   operators::QueryNodePtr map_owner = std::move(node.child);
   auto new_order_by = std::make_unique<operators::OrderByNode>(
      std::move(map.child), std::move(node.fields), node.randomize_seed
   );
   map.child = std::move(new_order_by);
   return map_owner;
}

}  // namespace silo::query_engine::optimizer
