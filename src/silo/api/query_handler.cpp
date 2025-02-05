#include "silo/api/query_handler.h"

#include <cxxabi.h>
#include <string>

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StreamCopier.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/api/active_database.h"
#include "silo/api/error_request_handler.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/optimizer/query_plan_generator.h"

namespace silo::api {
using silo::query_engine::QueryResultEntry;

QueryHandler::QueryHandler(
   std::shared_ptr<ActiveDatabase> database_handle,
   config::QueryOptions query_options
)
    : query_options(std::move(query_options)),
      database_handle(database_handle) {}

void QueryHandler::post(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto database = database_handle->getActiveDatabase();

   const auto request_id = response.get("X-Request-Id");

   std::string query_string;
   std::istream& istream = request.stream();
   Poco::StreamCopier::copyToString(istream, query_string);

   SPDLOG_INFO("Request Id [{}] - received query: {}", request_id, query_string);

   try {
      query_engine::optimizer::QueryPlanGenerator query_plan_generator(database);

      std::ostream proxy_out_stream(nullptr);

      auto query = query_engine::Query::parseQuery(query_string);
      auto query_plan =
         query_plan_generator.createQueryPlan(query, proxy_out_stream, query_options);

      response.set("data-version", database->getDataVersionTimestamp().value);
      response.setContentType("application/x-ndjson");
      std::ostream& out_stream = response.send();

      // Redirect proxy_out_stream to out_stream's buffer
      proxy_out_stream.rdbuf(out_stream.rdbuf());

      query_plan.execute();

   } catch (const silo::BadRequest& ex) {
      response.setContentType("application/json");
      SPDLOG_INFO("Query is invalid: {}", query_string, ex.what());
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{.error = "Bad request", .message = ex.what()});
   }
}

}  // namespace silo::api
