#include "admin_query_handler.h"

#include <string>
#include <utility>

#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/StreamCopier.h>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include <rhydb/append/append_exception.h>
#include <rhydb/query_engine/illegal_query_exception.h>
#include <rhydb/query_engine/saneql/parse_exception.h>
#include <evobench/evobench.hpp>

#include "active_database.h"
#include "bad_request.h"

namespace rhydb_app {

AdminQueryHandler::AdminQueryHandler(std::shared_ptr<ActiveDatabase> database_handle)
    : database_handle(std::move(database_handle)) {}

void AdminQueryHandler::post(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   EVOBENCH_SCOPE("AdminQueryHandler", "post");

   // Pin the database so it outlives the append, mirroring QueryHandler.
   const auto database = database_handle->getActiveDatabase();

   const auto request_id = response.get("X-Request-Id");

   std::string query_string;
   std::istream& istream = request.stream();
   Poco::StreamCopier::copyToString(istream, query_string);

   SPDLOG_INFO("Request Id [{}] - received admin query: {}", request_id, query_string);

   try {
      const nlohmann::json result = database->executeWrite(query_string);

      SPDLOG_INFO(
         "Request Id [{}] - admin write statement applied: {}; new data version {}",
         request_id,
         result.dump(),
         database->getDataVersionTimestamp().value
      );

      response.set("data-version", database->getDataVersionTimestamp().value);
      response.setContentType("application/json");
      std::ostream& output_stream = response.send();
      output_stream << result.dump();
   } catch (const rhydb::query_engine::saneql::ParseException& ex) {
      throw BadRequest(ex.what());
   } catch (const rhydb::query_engine::IllegalQueryException& ex) {
      throw BadRequest(ex.what());
   } catch (const rhydb::append::AppendException& ex) {
      throw BadRequest(ex.what());
   }
}

}  // namespace rhydb_app
