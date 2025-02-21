#include "silo/query_engine/query.h"

#include <string>

#include <nlohmann/json.hpp>

#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/filter_expressions/expression.h"

namespace silo::query_engine {

Query::Query(const std::string& query_string) {
   try {
      nlohmann::json json = nlohmann::json::parse(query_string);
      if (!json.contains("filterExpression") || !json["filterExpression"].is_object() ||
          !json.contains("action") || !json["action"].is_object()) {
         throw BadRequest("Query json must contain filterExpression and action.");
      }
      filter = json["filterExpression"]
                  .get<std::unique_ptr<silo::query_engine::filter_expressions::Expression>>();
      action = json["action"].get<std::unique_ptr<silo::query_engine::actions::Action>>();
   } catch (const nlohmann::json::parse_error& ex) {
      throw BadRequest("The query was not a valid JSON: " + std::string(ex.what()));
   } catch (const nlohmann::json::exception& ex) {
      throw BadRequest("The query was not a valid JSON: " + std::string(ex.what()));
   }
}

}  // namespace silo::query_engine