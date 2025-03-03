#include "silo/api/api.h"

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <spdlog/spdlog.h>

#include "silo/api/active_database.h"
#include "silo/api/request_handler_factory.h"
#include "silo/api/silo_directory_watcher.h"
#include "silo/common/silo_directory.h"

namespace silo::api {

int Api::runApi(const silo::config::RuntimeConfig& runtime_config) {
   SPDLOG_INFO("Starting SILO API");

   const Poco::Net::ServerSocket server_socket(runtime_config.api_options.port);

   auto* const poco_parameter = new Poco::Net::HTTPServerParams;

   SPDLOG_INFO("Using {} queued http connections", runtime_config.api_options.max_connections);
   poco_parameter->setMaxQueued(runtime_config.api_options.max_connections);

   SPDLOG_INFO(
      "Using {} threads for http connections", runtime_config.api_options.parallel_threads
   );
   poco_parameter->setMaxThreads(runtime_config.api_options.parallel_threads);

   // For better profiling, we do not want requests to allocate new threads in the thread pool.
   // Instead, just allocate all of them directly on start-up (by setting minCapacity)
   Poco::ThreadPool thread_pool(
      /* minCapacity = */ runtime_config.api_options.parallel_threads,
      /* maxCapacity = */ runtime_config.api_options.parallel_threads
   );

   auto database = std::make_shared<ActiveDatabase>();

   auto silo_request_handler_factory =
      std::make_unique<silo::api::SiloRequestHandlerFactory>(runtime_config, database);

   const silo::api::SiloDirectoryWatcher watcher(SiloDirectory{runtime_config.data_directory}, database);

   // HTTPServer will erase the memory of the request_handler, therefore we call `release`
   Poco::Net::HTTPServer server(
      silo_request_handler_factory.release(), thread_pool, server_socket, poco_parameter
   );

   SPDLOG_INFO("Listening on port {}", runtime_config.api_options.port);

   server.start();
   waitForTerminationRequest();
   server.stop();

   return Application::EXIT_OK;
}

}  // namespace silo::api
