#include "silo/query_engine/optimizer/map_pullup_pass.h"

#include <string>
#include <unordered_set>

#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/filter_node.h"
#include "silo/query_engine/operators/map_node.h"

namespace silo::query_engine::optimizer {

namespace {

std::unordered_set<std::string> producedColumns(const operators::MapNode& node) {
   std::unordered_set<std::string> produced;
   for (const auto& assignment : node.assignments) {
      produced.insert(assignment.output_column.name);
   }
   return produced;
}

std::unordered_set<std::string> requiredColumns(const operators::FilterNode& node) {
   std::unordered_set<std::string> required;
   for (const auto& column : node.filter->freeIUs()) {
      required.insert(column.name);
   }
   return required;
}

bool intersects(
   const std::unordered_set<std::string>& lhs,
   const std::unordered_set<std::string>& rhs
) {
   for (const auto& value : lhs) {
      if (rhs.contains(value)) {
         return true;
      }
   }
   return false;
}

std::unique_ptr<operators::MapNode> getMapChildOrNone(operators::QueryNodePtr& child) {
   if (dynamic_cast<operators::MapNode*>(child.get()) != nullptr) {
      return std::unique_ptr<operators::MapNode>(child.release());
   }
   return nullptr;
}

}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr MapPullupPass::operator()(operators::FetchNode& node) {
   propagateToNode(node.child);

   auto map_child = getMapChildOrNone(node.child);
   if (!map_child) {
      return nullptr;
   }
   map_child->child =
      std::make_unique<operators::FetchNode>(std::move(map_child->child), node.count, node.offset);
   return map_child;
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr MapPullupPass::operator()(operators::FilterNode& node) {
   propagateToNode(node.child);

   auto map_child = getMapChildOrNone(node.child);
   if (!map_child || intersects(producedColumns(*map_child), requiredColumns(node))) {
      return nullptr;
   }
   map_child->child =
      std::make_unique<operators::FilterNode>(std::move(map_child->child), std::move(node.filter));
   return map_child;
}

}  // namespace silo::query_engine::optimizer
