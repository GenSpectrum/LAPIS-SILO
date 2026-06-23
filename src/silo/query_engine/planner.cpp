#include "silo/query_engine/planner.h"

#include <stdexcept>

#include <arrow/acero/exec_plan.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/column_narrowing_pass.h"
#include "silo/query_engine/filter_pushdown_pass.h"
#include "silo/query_engine/node_resolution_pass.h"
#include "silo/query_engine/saneql/ast_to_query.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine {

namespace {

arrow::Result<QueryPlan> planQueryOrError(
   const operators::QueryNode& node,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());
   ARROW_ASSIGN_OR_RAISE(auto* top_node, node.addToExecPlan(*arrow_plan, tables, query_options));
   return QueryPlan::makeQueryPlan(std::move(arrow_plan), top_node, request_id);
}

}  // namespace

QueryPlan Planner::planQuery(
   operators::QueryNodePtr node,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   auto log_plan = [&](std::string_view phase) {
      if (spdlog::should_log(spdlog::level::debug)) {
         SPDLOG_DEBUG("[{}] {}: {}", request_id, phase, node->toJson().dump());
      }
   };
   log_plan("initial");
   node = ColumnNarrowingPass::run(std::move(node));
   log_plan("after ColumnNarrowingPass");
   node = FilterPushdownPass::run(std::move(node));
   log_plan("after FilterPushdownPass");
   node = NodeResolutionPass::run(std::move(node));
   log_plan("after NodeResolutionPass");
   auto result = planQueryOrError(*node, tables, query_options, request_id);
   if (!result.ok()) {
      throw std::runtime_error(
         fmt::format("Error when planning query execution: {}", result.status().ToString())
      );
   }
   return std::move(result.ValueUnsafe());
}

QueryPlan Planner::planSaneqlQuery(
   std::string_view query_string,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   auto query_node = saneql::parseAndConvertToQueryTree(query_string, tables);
   return planQuery(std::move(query_node), tables, query_options, request_id);
}

}  // namespace silo::query_engine
