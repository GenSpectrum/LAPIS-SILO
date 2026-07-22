#include "request_handler_factory.h"

#include <string>
#include <utility>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>

#include "error_request_handler.h"
#include "health_handler.h"
#include "info_handler.h"
#include "lineage_definition_handler.h"
#include "logging_request_handler.h"
#include "not_found_handler.h"
#include "query_handler.h"
#include "request_id_handler.h"

namespace silo_app {

SiloRequestHandlerFactory::SiloRequestHandlerFactory(
   silo::config::RuntimeConfig runtime_config,
   std::shared_ptr<ActiveDatabase> database_handle
)
    : runtime_config(std::move(runtime_config)),
      database_handle(std::move(database_handle)) {}

Poco::Net::HTTPRequestHandler* SiloRequestHandlerFactory::createRequestHandler(
   const Poco::Net::HTTPServerRequest& request
) {
   return new RequestIdHandler(
      std::make_unique<LoggingRequestHandler>(std::make_unique<ErrorRequestHandler>(
         routeRequest(Poco::URI(request.getURI())), runtime_config
      ))
   );
}

std::unique_ptr<Poco::Net::HTTPRequestHandler> SiloRequestHandlerFactory::routeRequest(
   const Poco::URI& uri
) {
   const std::string_view path = uri.getPath();
   std::vector<std::string> segments;
   uri.getPathSegments(segments);

   if (path == "/health") {
      return std::make_unique<silo_app::HealthHandler>();
   }
   if (path == "/info") {
      return std::make_unique<silo_app::InfoHandler>(database_handle);
   }
   if (segments.size() == 2 && segments.at(0) == "lineageDefinition") {
      return std::make_unique<silo_app::LineageDefinitionHandler>(database_handle, segments.at(1));
   }
   if (path == "/query") {
      return std::make_unique<silo_app::QueryHandler>(
         database_handle, runtime_config.query_options
      );
   }
   return std::make_unique<silo_app::NotFoundHandler>();
}

}  // namespace silo_app
