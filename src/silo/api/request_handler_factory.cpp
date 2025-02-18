#include "silo/api/request_handler_factory.h"

#include <string>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>

#include "silo/api/error_request_handler.h"
#include "silo/api/info_handler.h"
#include "silo/api/lineage_definition_handler.h"
#include "silo/api/logging_request_handler.h"
#include "silo/api/not_found_handler.h"
#include "silo/api/query_handler.h"
#include "silo/api/request_id_handler.h"

namespace silo::api {

SiloRequestHandlerFactory::SiloRequestHandlerFactory(
   silo::config::RuntimeConfig runtime_config,
   std::shared_ptr<ActiveDatabase> database_handle
)
    : runtime_config(std::move(runtime_config)),
      database_handle(database_handle) {}

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
   const auto path = uri.getPath();
   std::vector<std::string> segments;
   uri.getPathSegments(segments);

   if (path == "/info") {
      return std::make_unique<silo::api::InfoHandler>(database_handle->getActiveDatabase());
   }
   if (segments.size() == 2 && segments.at(0) == "lineageDefinition") {
      return std::make_unique<silo::api::LineageDefinitionHandler>(
         database_handle->getActiveDatabase(), segments.at(1)
      );
   }
   if (path == "/query") {
      return std::make_unique<silo::api::QueryHandler>(database_handle->getActiveDatabase());
   }
   return std::make_unique<silo::api::NotFoundHandler>();
}

}  // namespace silo::api
