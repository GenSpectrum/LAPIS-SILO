#include "silo/query_engine/query.h"

#include <string>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"

using silo::query_engine::actions::Action;
using silo::query_engine::filter::expressions::Expression;

namespace silo::query_engine {

std::shared_ptr<Query> Query::parseQuery(const std::string& query_string) {
   try {
      nlohmann::json json = nlohmann::json::parse(query_string);
      if (!json.contains("filterExpression") || !json["filterExpression"].is_object() ||
          !json.contains("action") || !json["action"].is_object()) {
         throw BadRequest("Query json must contain filterExpression and action.");
      }
      auto filter = json["filterExpression"].get<std::unique_ptr<Expression>>();
      auto action = json["action"].get<std::unique_ptr<Action>>();
      return std::make_shared<Query>(std::move(filter), std::move(action));
   } catch (const nlohmann::json::parse_error& ex) {
      throw BadRequest("The query was not a valid JSON: " + std::string(ex.what()));
   } catch (const nlohmann::json::exception& ex) {
      throw BadRequest("The query was not a valid JSON: " + std::string(ex.what()));
   }
}

QueryPlan Query::toQueryPlan(
   std::shared_ptr<Database> database,
   const config::QueryOptions& query_options,
   std::string_view request_id
) const {
   SPDLOG_DEBUG("Request Id [{}] - Parsed filter: {}", request_id, filter->toString());

   std::vector<CopyOnWriteBitmap> partition_filters;
   partition_filters.reserve(database->table->getNumberOfPartitions());
   for (size_t partition_index = 0; partition_index < database->table->getNumberOfPartitions();
        partition_index++) {
      auto filter_after_rewrite = filter->rewrite(
         *database->table,
         database->table->getPartition(partition_index),
         Expression::AmbiguityMode::NONE
      );
      SPDLOG_DEBUG(
         "Request Id [{}] - Filter after rewrite for partition {}: {}",
         request_id,
         partition_index,
         filter_after_rewrite->toString()
      );
      auto filter_operator = filter_after_rewrite->compile(
         *database->table, database->table->getPartition(partition_index)
      );
      SPDLOG_DEBUG(
         "Request Id [{}] - Filter operator tree for partition {}: {}",
         request_id,
         partition_index,
         filter_operator->toString()
      );
      partition_filters.emplace_back(filter_operator->evaluate());
   };

   return action->toQueryPlan(database->table, partition_filters, query_options, request_id);
}

}  // namespace silo::query_engine