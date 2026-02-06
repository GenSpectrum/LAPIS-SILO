#include "silo/api/query_handler.h"

#include <cxxabi.h>
#include <string>

#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StreamCopier.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <utility>

#include "evobench/evobench.hpp"
#include "silo/api/active_database.h"
#include "silo/api/bad_request.h"
#include "silo/api/error_request_handler.h"
#include "silo/query_engine/exec_node/ndjson_sink.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/query.h"

namespace silo::api {

QueryHandler::QueryHandler(
   std::shared_ptr<ActiveDatabase> database_handle,
   config::QueryOptions query_options
)
    : query_options(query_options),
      database_handle(std::move(database_handle)) {}

namespace {

const uint64_t DEFAULT_TIMEOUT_TWO_MINUTES = 120;

}

void QueryHandler::post(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   EVOBENCH_SCOPE("QueryHandler", "post");

   // This fixes the database to outlive the execution of the query
   const auto database = database_handle->getActiveDatabase();

   const auto request_id = response.get("X-Request-Id");

   std::string query_string;
   std::istream& istream = request.stream();
   Poco::StreamCopier::copyToString(istream, query_string);

   SPDLOG_INFO("Request Id [{}] - received query: {}", request_id, query_string);

   try {
      auto query = query_engine::Query::parseQuery(query_string);
      auto query_plan = database->createQueryPlan(*query, query_options, request_id);

      response.set("data-version", database->getDataVersionTimestamp().value);
      response.setContentType("application/x-ndjson");
      std::ostream& output_stream = response.send();
      query_engine::exec_node::NdjsonSink output_sink{&output_stream, query_plan.results_schema};

      // This function is not inside executeAndWrite, because we need the context from query->action
      EVOBENCH_SCOPE("QueryPlan", "executeAndWrite");
      EVOBENCH_KEY_VALUE("of query_type", query->action->getType());
      query_plan.executeAndWrite(output_sink, DEFAULT_TIMEOUT_TWO_MINUTES);
   } catch (const silo::query_engine::IllegalQueryException& ex) {
      throw BadRequest(ex.what());
   }
}

}  // namespace silo::api
