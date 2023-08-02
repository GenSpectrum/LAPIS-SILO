#include "silo_api/request_handler_factory.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>

#include "silo/database.h"
#include "silo_api/error_request_handler.h"
#include "silo_api/info_handler.h"
#include "silo_api/logging_request_handler.h"
#include "silo_api/not_found_handler.h"
#include "silo_api/query_handler.h"

namespace silo_api {

SiloRequestHandlerFactory::SiloRequestHandlerFactory(
   const silo::Database& database,
   const silo::query_engine::QueryEngine& query_engine
)
    : database(database),
      query_engine(query_engine) {}

Poco::Net::HTTPRequestHandler* SiloRequestHandlerFactory::createRequestHandler(
   const Poco::Net::HTTPServerRequest& request
) {
   return new silo_api::LoggingRequestHandler(new silo_api::ErrorRequestHandler(routeRequest(request
   )));
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
      return new silo_api::QueryHandler(query_engine, database.getDataVersion().toString());
   }
   return new silo_api::NotFoundHandler;
}

}  // namespace silo_api
