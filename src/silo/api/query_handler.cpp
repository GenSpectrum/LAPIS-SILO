#include "silo/api/query_handler.h"

#include <cxxabi.h>
#include <string>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/api/active_database.h"
#include "silo/query_engine/bad_request.h"

namespace silo::api {

void QueryHandler::post(
   std::shared_ptr<const Database> database,
   crow::request& request,
   crow::response& response
) {
   const auto request_id = response.get_header_value("X-Request-Id");

   std::string query = request.body;

   SPDLOG_INFO("Request Id [{}] - received query: {}", request_id, query);

   try {
      auto query_result = database->executeQuery(query);

      response.set_header("data-version", database->getDataVersionTimestamp().value);

      response.set_header("Content-Type", "application/x-ndjson");
      std::optional<std::reference_wrapper<const silo::query_engine::QueryResultEntry>> entry;
      while ((entry = query_result.next())) {
         // TODO make streaming
         response.body += nlohmann::json(*entry).dump() + "\n";
      }
      response.end();
   } catch (const silo::BadRequest& ex) {
      response.set_header("Content-Type", "application/json");
      SPDLOG_INFO("Query is invalid: {}", query, ex.what());
      response.code = crow::BAD_REQUEST;
      response.body = nlohmann::json{{"error", "Bad request"}, {"message", ex.what()}}.dump();
      // response.body = nlohmann::json{{"error", "BadRequest"}, {"message", ex.what()}};
      response.end();
   }
}

}  // namespace silo::api
