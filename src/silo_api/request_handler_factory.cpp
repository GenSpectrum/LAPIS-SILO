#include "silo_api/request_handler_factory.h"

#include <string>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>

#include "silo_api/error_request_handler.h"
#include "silo_api/info_handler.h"
#include "silo_api/logging_request_handler.h"
#include "silo_api/not_found_handler.h"
#include "silo_api/query_handler.h"
#include "silo_api/request_id_handler.h"

using silo_api::ErrorRequestHandler;
using silo_api::LoggingRequestHandler;
using silo_api::RequestIdHandler;

namespace silo_api {

SiloRequestHandlerFactory::SiloRequestHandlerFactory(
   silo_api::DatabaseMutex& database,
   silo::config::RuntimeConfig runtime_config
)
    : database(database),
      runtime_config(std::move(runtime_config)) {}

Poco::Net::HTTPRequestHandler* SiloRequestHandlerFactory::createRequestHandler(
   const Poco::Net::HTTPServerRequest& request
) {
   return new RequestIdHandler(
      new LoggingRequestHandler(new ErrorRequestHandler(routeRequest(request), runtime_config))
   );
}

Poco::Net::HTTPRequestHandler* SiloRequestHandlerFactory::routeRequest(
   const Poco::Net::HTTPServerRequest& request
) {
   const auto& uri = Poco::URI(request.getURI());
   const auto path = uri.getPath();
   if (path == "/info") {
      return new silo_api::InfoHandler(database);
   }
   if (path == "/query") {
      return new silo_api::QueryHandler(database);
   }
   return new silo_api::NotFoundHandler;
}

}  // namespace silo_api
