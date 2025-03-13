#include "silo/api/api.h"

#include <crow.h>
#include <spdlog/spdlog.h>

#include "silo/api/active_database.h"
#include "silo/api/crow_spdlog_adapter.h"
#include "silo/api/database_directory_watcher.h"
#include "silo/api/info_handler.h"
#include "silo/api/lineage_definition_handler.h"
#include "silo/api/logging_request_middleware.h"
#include "silo/api/query_handler.h"
#include "silo/api/request_handler_factory.h"
#include "silo/api/request_id_middleware.h"

namespace silo::api {

int Api::runApi(const silo::config::RuntimeConfig& runtime_config) {
   SPDLOG_INFO("Starting SILO API");
   CrowSpdlogAdapter logger;
   crow::logger::setHandler(&logger);
   crow::logger::setLogLevel(crow::LogLevel::CRITICAL);

   auto active_database = std::make_shared<ActiveDatabase>();

   silo::api::DatabaseDirectoryWatcher watcher(runtime_config.data_directory, active_database);
   watcher.start();

   crow::App<RequestIdMiddleware, LoggingRequestMiddleware> app;

   //   if (path == "/info") {
   //      return std::make_unique<silo::api::InfoHandler>(database);
   //   }
   //   if (segments.size() == 2 && segments.at(0) == "lineageDefinition") {
   //      return std::make_unique<silo::api::LineageDefinitionHandler>(database, segments.at(1));
   //   }
   //   if (path == "/query") {
   //      return std::make_unique<silo::api::QueryHandler>(database);
   //   }
   //   return std::make_unique<silo::api::NotFoundHandler>();
   CROW_ROUTE(app, "/info")
   ([&](crow::request& request, crow::response& response) {
      InfoHandler::get(active_database->getActiveDatabase(), request, response);
   });

   CROW_ROUTE(app, "/lineageDefinition/<string>")
   ([&](crow::request& request, crow::response& response, const std::string& column_name) {
      LineageDefinitionHandler::get(
         active_database->getActiveDatabase(), request, response, column_name
      );
   });

   CROW_ROUTE(app, "/query")
      .methods(crow::HTTPMethod::POST)([&](crow::request& request, crow::response& response) {
         QueryHandler::post(active_database->getActiveDatabase(), request, response);
      });

   app.bindaddr("127.0.0.1").port(runtime_config.api_options.port).concurrency(4).run();

   watcher.stop();

   return 0;
}

}  // namespace silo::api
