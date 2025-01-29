#include "silo/api/query_handler.h"

#include <cxxabi.h>
#include <string>

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StreamCopier.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/api/database_mutex.h"
#include "silo/api/error_request_handler.h"
#include "silo/query_engine/query_parse_exception.h"

namespace silo::api {
using silo::query_engine::QueryResultEntry;

QueryHandler::QueryHandler(silo::api::DatabaseMutex& database_mutex)
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

      auto query_result = fixed_database->executeQuery(query);

      response.set("data-version", fixed_database->getDataVersionTimestamp().value);

      response.setContentType("application/x-ndjson");
      std::ostream& out_stream = response.send();
      std::optional<std::reference_wrapper<const QueryResultEntry>> entry;
      while ((entry = query_result.next())) {
         out_stream << nlohmann::json(*entry) << '\n';
         if (!out_stream) {
            throw std::runtime_error(
               fmt::format("error writing to HTTP stream: {}", std::strerror(errno))
            );
         }
      }
   } catch (const silo::QueryException& ex) {
      response.setContentType("application/json");
      SPDLOG_INFO(
         "Query is invalid: {} - exception during {}: {}", query, ex.duringString(), ex.what()
      );
      response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{.error = "Bad request", .message = ex.what()});
   }
}

}  // namespace silo::api
