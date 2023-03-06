#include <nlohmann/json.hpp>
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "silo/database.h"
#include "silo/query_engine/query_engine.h"
#include "silo_api/error.h"
#include "silo_api/query_handler.h"
#include "Poco/StreamCopier.h"
#include "silo_api/variant_json_serializer.h"

namespace silo {
namespace response{
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(aggregation_result, count);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(mutation_proportion, mutation, proportion, count);
}
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(result_s, queryResult, parseTime, filterTime, actionTime);
}

namespace silo_api {
void QueryHandler::handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response) {
      std::string query;
      std::istream& istream = request.stream();
      Poco::StreamCopier::copyToString(istream, query);

      response.setContentType("application/json");

      try {
         const auto query_result = silo::execute_query(database, query, std::cout, std::cout, std::cout);

         std::ostream& out_stream = response.send();
         out_stream << nlohmann::json(query_result);
      } catch (const silo::QueryParseException& ex) {
         response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
         std::ostream& out_stream = response.send();
         out_stream << nlohmann::json(ErrorResponse{"Bad request", ex.what()});
      } catch (const std::exception& ex) {
         response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
         std::ostream& out_stream = response.send();
         out_stream << nlohmann::json(ErrorResponse{"Internal server error", ex.what()});
      }
}
}