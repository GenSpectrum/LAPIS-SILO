#include "silo/query_engine/planner.h"

#include "silo/query_engine/operators/aggregate_node.h"
#include "silo/query_engine/operators/count_filter_node.h"
#include "silo/query_engine/operators/fetch_node.h"
#include "silo/query_engine/operators/insertions_node.h"
#include "silo/query_engine/operators/most_recent_common_ancestor_node.h"
#include "silo/query_engine/operators/mutations_node.h"
#include "silo/query_engine/operators/order_by_node.h"
#include "silo/query_engine/operators/table_scan_node.h"

namespace silo::query_engine {

namespace {

arrow::Result<QueryPlan> planQueryOrError(
   const silo::query_engine::operators::QueryNode& node,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   ARROW_ASSIGN_OR_RAISE(auto partial_query_plan, node.toQueryPlan(tables, query_options));
   return query_engine::QueryPlan::makeQueryPlan(
      partial_query_plan.plan, partial_query_plan.top_node, request_id
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr optimizeInstance(operators::AggregateNode* node) {
   // Full aggregations (COUNT(*) and only a filter below can be optimized)
   if (node->group_by_fields.empty() &&
       dynamic_cast<operators::TableScanNode*>(node->child.get()) != nullptr) {
      auto* table_scan_child = dynamic_cast<operators::TableScanNode*>(node->child.get());
      return std::make_unique<operators::CountFilterNode>(
         std::move(table_scan_child->table), std::move(table_scan_child->filter)
      );
   }
   return std::make_unique<operators::AggregateNode>(
      Planner::optimize(std::move(node->child)), std::move(node->group_by_fields)
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr optimizeInstance(operators::OrderByNode* node) {
   return std::make_unique<operators::OrderByNode>(
      Planner::optimize(std::move(node->child)), std::move(node->fields), node->randomize_seed
   );
}

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr optimizeInstance(operators::FetchNode* node) {
   return std::make_unique<operators::FetchNode>(
      Planner::optimize(std::move(node->child)), node->count, node->offset
   );
}

}  // namespace

// NOLINTNEXTLINE(misc-no-recursion)
operators::QueryNodePtr Planner::optimize(operators::QueryNodePtr node) {
   if (dynamic_cast<operators::AggregateNode*>(node.get()) != nullptr) {
      auto* node_instance = dynamic_cast<operators::AggregateNode*>(node.get());
      return optimizeInstance(node_instance);
   }
   if (dynamic_cast<operators::OrderByNode*>(node.get()) != nullptr) {
      auto* node_instance = dynamic_cast<operators::OrderByNode*>(node.get());
      return optimizeInstance(node_instance);
   }
   if (dynamic_cast<operators::FetchNode*>(node.get()) != nullptr) {
      auto* node_instance = dynamic_cast<operators::FetchNode*>(node.get());
      return optimizeInstance(node_instance);
   }
   return node;
}

QueryPlan Planner::planQuery(
   operators::QueryNodePtr node,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   auto optimized_tree = optimize(std::move(node));
   auto result = planQueryOrError(*optimized_tree, tables, query_options, request_id);
   if (!result.ok()) {
      throw std::runtime_error(
         fmt::format("Error when planning query execution: {}", result.status().ToString())
      );
   }
   return std::move(result.ValueUnsafe());
}

}  // namespace silo::query_engine
