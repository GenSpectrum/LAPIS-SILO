#include "silo/query_engine/query.h"

#include <string>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/illegal_query_exception.h"

using silo::query_engine::actions::Action;
using silo::query_engine::filter::expressions::Expression;

namespace silo::query_engine {

std::shared_ptr<Query> Query::parseQuery(const std::string& query_string) {
   try {
      nlohmann::json json = nlohmann::json::parse(query_string);
      if (!json.contains("filterExpression") || !json["filterExpression"].is_object() ||
          !json.contains("action") || !json["action"].is_object()) {
         throw IllegalQueryException("Query json must contain filterExpression and action.");
      }
      auto filter = json["filterExpression"].get<std::unique_ptr<Expression>>();
      auto action = json["action"].get<std::unique_ptr<Action>>();
      return std::make_shared<Query>(std::move(filter), std::move(action));
   } catch (const nlohmann::json::parse_error& ex) {
      throw IllegalQueryException("The query was not a valid JSON: " + std::string(ex.what()));
   } catch (const nlohmann::json::exception& ex) {
      throw IllegalQueryException("The query was not a valid JSON: " + std::string(ex.what()));
   }
}

}  // namespace silo::query_engine