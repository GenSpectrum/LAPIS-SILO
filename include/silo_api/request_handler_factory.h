#ifndef SILO_REQUEST_HANDLER_FACTORY_H
#define SILO_REQUEST_HANDLER_FACTORY_H

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

namespace silo {
class Database;
class QueryEngine;
}  // namespace silo

namespace silo_api {

class SiloRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
  private:
   const silo::Database& database;
   const silo::QueryEngine& query_engine;

  public:
   SiloRequestHandlerFactory(const silo::Database& database, const silo::QueryEngine& query_engine);

   Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);

  private:
   Poco::Net::HTTPRequestHandler* routeRequest(const Poco::Net::HTTPServerRequest& request);
};

}  // namespace silo_api

#endif  // SILO_REQUEST_HANDLER_FACTORY_H
