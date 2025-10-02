#include "silo/api/api.h"

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/ServerSocket.h>
#include <spdlog/spdlog.h>

#include "silo/api/active_database.h"
#include "silo/api/memory_monitor.h"
#include "silo/api/request_handler_factory.h"
#include "silo/api/silo_directory_watcher.h"
#include "silo/common/silo_directory.h"

namespace silo::api {

int Api::runApi(const silo::config::RuntimeConfig& runtime_config) {
   SPDLOG_INFO("Starting SILO API");

   Poco::Net::SocketAddress address(runtime_config.api_options.port);

   Poco::Net::ServerSocket server_socket;
   // Set timeouts to avoid hanging connections
   server_socket.setReceiveTimeout(Poco::Timespan(30,0));
   server_socket.setSendTimeout(Poco::Timespan(120,0));
   try {
      server_socket.bind(address, true);
      server_socket.listen();
   } catch (const Poco::Net::NetException& e) {
      SPDLOG_ERROR(
         "Failed to bind to port {}: {}", runtime_config.api_options.port, e.displayText()
      );
      return EXIT_FAILURE;
   }

   auto* const poco_parameter = new Poco::Net::HTTPServerParams;

   poco_parameter->setKeepAlive(true);
   // close idle connections after 10 seconds
   poco_parameter->setKeepAliveTimeout(Poco::Timespan(10, 0));

   SPDLOG_INFO("Using {} queued http connections", runtime_config.api_options.max_connections);
   poco_parameter->setMaxQueued(runtime_config.api_options.max_connections);

   auto worker_threads_to_use = runtime_config.api_options.parallel_threads;
   if (worker_threads_to_use == 0) {
      worker_threads_to_use = std::thread::hardware_concurrency();
   }
   SPDLOG_INFO("Using {} threads for http connections", worker_threads_to_use);
   poco_parameter->setMaxThreads(worker_threads_to_use);

   // For better profiling, we do not want requests to allocate new threads in the thread pool.
   // Instead, just allocate all of them directly on start-up (by setting minCapacity)
   Poco::ThreadPool thread_pool(
      /* minCapacity = */ worker_threads_to_use,
      /* maxCapacity = */ worker_threads_to_use
   );

   auto database = std::make_shared<ActiveDatabase>();

   auto silo_request_handler_factory =
      std::make_unique<silo::api::SiloRequestHandlerFactory>(runtime_config, database);

   const silo::api::SiloDirectoryWatcher directory_watcher(
      SiloDirectory{runtime_config.data_directory}, database
   );

   const silo::api::MemoryMonitor memory_monitor{runtime_config.api_options.soft_memory_limit};

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
