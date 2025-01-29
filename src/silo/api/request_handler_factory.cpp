#include "silo/api/request_handler_factory.h"

#include <string>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>

#include "silo/api/error_request_handler.h"
#include "silo/api/info_handler.h"
#include "silo/api/logging_request_handler.h"
#include "silo/api/not_found_handler.h"
#include "silo/api/query_handler.h"
#include "silo/api/request_id_handler.h"

namespace silo::api {

SiloRequestHandlerFactory::SiloRequestHandlerFactory(
   silo::api::DatabaseMutex& database,
   silo::config::RuntimeConfig runtime_config
)
    : database(database),
      runtime_config(std::move(runtime_config)) {}

Poco::Net::HTTPRequestHandler* SiloRequestHandlerFactory::createRequestHandler(
   const Poco::Net::HTTPServerRequest& request
) {
   return new RequestIdHandler(std::make_unique<LoggingRequestHandler>(
      std::make_unique<ErrorRequestHandler>(routeRequest(request), runtime_config)
   ));
}

std::unique_ptr<Poco::Net::HTTPRequestHandler> SiloRequestHandlerFactory::routeRequest(
   const Poco::Net::HTTPServerRequest& request
) {
   const auto& uri = Poco::URI(request.getURI());
   const auto path = uri.getPath();
   if (path == "/info") {
      return std::make_unique<silo::api::InfoHandler>(database);
   }
   if (path == "/query") {
      return std::make_unique<silo::api::QueryHandler>(database);
   }
   return std::make_unique<silo::api::NotFoundHandler>();
}

}  // namespace silo::api
