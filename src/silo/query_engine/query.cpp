#include "silo/query_engine/query.h"

#include <string>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

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
   std::ostream& output_stream,
   const config::QueryOptions& query_options
) const {
   SPDLOG_DEBUG("Parsed filter: {}", filter->toString());

   std::vector<std::unique_ptr<filter::operators::Operator>> partition_filter_operators;
   partition_filter_operators.reserve(database->table->getNumberOfPartitions());
   for (size_t partition_index = 0; partition_index < database->table->getNumberOfPartitions();
        partition_index++) {
      partition_filter_operators.emplace_back(filter->compile(
         *database,
         database->table->getPartition(partition_index),
         filter::expressions::Expression::AmbiguityMode::NONE
      ));
      SPDLOG_DEBUG(
         "Simplified query for partition {}: {}",
         partition_index,
         partition_filter_operators.back()->toString()
      );
   };

   return action->toQueryPlan(
      database->table, partition_filter_operators, output_stream, query_options
   );
}

}  // namespace silo::query_engine