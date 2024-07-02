#include "silo_api/query_handler.h"

#include <cxxabi.h>
#include <string>

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StreamCopier.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/query_parse_exception.h"
#include "silo_api/database_mutex.h"
#include "silo_api/error_request_handler.h"

namespace silo_api {

QueryHandler::QueryHandler(silo_api::DatabaseMutex& database_mutex)
    : database_mutex(database_mutex) {}

void QueryHandler::post(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto request_id = response.get("X-Request-Id");

   std::string query;
   std::istream& istream = request.stream();
   Poco::StreamCopier::copyToString(istream, query);

   SPDLOG_INFO("Request Id [{}] - received query: {}", request_id, query);

   try {
      const auto fixed_database = database_mutex.getDatabase();

      const auto query_result = fixed_database.database.executeQuery(query);

      response.set("data-version", fixed_database.database.getDataVersionTimestamp().value);

      response.setContentType("application/x-ndjson");
      std::ostream& out_stream = response.send();
      for (const auto& entry : query_result.entries()) {
         out_stream << nlohmann::json(entry) << '\n';
      }
   } catch (const silo::QueryParseException& ex) {
      response.setContentType("application/json");
      SPDLOG_INFO("Query is invalid: " + query + " - exception: " + ex.what());
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{.error = "Bad request", .message = ex.what()});
   }
}

}  // namespace silo_api
