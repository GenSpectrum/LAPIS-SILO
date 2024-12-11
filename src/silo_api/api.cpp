#include "silo_api/api.h"

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <spdlog/spdlog.h>

#include "silo_api/database_directory_watcher.h"
#include "silo_api/database_mutex.h"
#include "silo_api/request_handler_factory.h"

int SiloServer::runApi(const silo::config::RuntimeConfig& runtime_config) {
   SPDLOG_INFO("Starting SILO API");

   silo_api::DatabaseMutex database_mutex;

   const Poco::Net::ServerSocket server_socket(runtime_config.api_options.port);

   const silo_api::DatabaseDirectoryWatcher watcher(runtime_config.data_directory, database_mutex);

   auto* const poco_parameter = new Poco::Net::HTTPServerParams;

   SPDLOG_INFO("Using {} queued http connections", runtime_config.api_options.max_connections);
   poco_parameter->setMaxQueued(runtime_config.api_options.max_connections);

   SPDLOG_INFO(
      "Using {} threads for http connections", runtime_config.api_options.parallel_threads
   );
   poco_parameter->setMaxThreads(runtime_config.api_options.parallel_threads);

   Poco::Net::HTTPServer server(
      new silo_api::SiloRequestHandlerFactory(database_mutex, runtime_config),
      server_socket,
      poco_parameter
   );

   SPDLOG_INFO("Listening on port {}", runtime_config.api_options.port);

   server.start();
   waitForTerminationRequest();
   server.stop();

   return Application::EXIT_OK;
}
