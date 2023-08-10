#include "silo_api/query_handler.h"

#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StreamCopier.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/database.h"
#include "silo/query_engine/query_engine.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo_api/database_mutex.h"
#include "silo_api/error_request_handler.h"

namespace silo_api {

QueryHandler::QueryHandler(silo_api::DatabaseMutex& database)
    : database(database) {}

void QueryHandler::post(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   std::string query;
   std::istream& istream = request.stream();
   Poco::StreamCopier::copyToString(istream, query);

   SPDLOG_INFO("received query: {}", query);

   response.setContentType("application/json");
   try {
      auto fixed_database = database.getDatabase();

      silo::query_engine::QueryEngine query_engine(fixed_database.database);

      const auto query_result = query_engine.executeQuery(query);

      response.set("data-version", fixed_database.database.getDataVersion().toString());

      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(query_result);
   } catch (const silo::QueryParseException& ex) {
      response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{"Bad request", ex.what()});
   }
}

}  // namespace silo_api