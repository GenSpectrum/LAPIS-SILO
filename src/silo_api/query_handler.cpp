#include "silo_api/query_handler.h"

#include <string>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StreamCopier.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "silo/query_engine/query_engine.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo_api/request_handler.h"
#include "silo_api/variant_json_serializer.h"

namespace silo::response {
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(AggregationResult, count);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MutationProportion, mutation, proportion, count);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResult, error, message);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(QueryResult, queryResult, parseTime, filterTime, actionTime);
}  // namespace silo::response

namespace silo_api {

QueryHandler::QueryHandler(const silo::QueryEngine& query_engine)
    : query_engine(query_engine) {}

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
      const auto query_result = query_engine.executeQuery(query);

      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(query_result);
   } catch (const silo::QueryParseException& ex) {
      response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
      std::ostream& out_stream = response.send();
      out_stream << nlohmann::json(ErrorResponse{"Bad request", ex.what()});
   }
}

}  // namespace silo_api