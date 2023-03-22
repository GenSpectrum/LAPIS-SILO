#include "silo_api/request_handler_factory.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>

#include "silo/database.h"
#include "silo_api/info_handler.h"
#include "silo_api/not_found_handler.h"
#include "silo_api/query_handler.h"
#include "silo_api/request_handler.h"

namespace silo_api {

SiloRequestHandlerFactory::SiloRequestHandlerFactory(
   const silo::Database& database,
   const silo::QueryEngine& query_engine
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
   if (request.getURI() == "/info") {
      return new silo_api::InfoHandler(database);
   }
   if (request.getURI() == "/query") {
      return new silo_api::QueryHandler(query_engine);
   }
   return new silo_api::NotFoundHandler;
}

}  // namespace silo_api
