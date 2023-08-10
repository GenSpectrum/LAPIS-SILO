#ifndef SILO_REQUEST_HANDLER_FACTORY_H
#define SILO_REQUEST_HANDLER_FACTORY_H

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

namespace silo_api {
class DatabaseMutex;
}  // namespace silo_api

namespace silo_api {

class SiloRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
  private:
   silo_api::DatabaseMutex& database;

  public:
   SiloRequestHandlerFactory(silo_api::DatabaseMutex& database);

   Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);

  private:
   Poco::Net::HTTPRequestHandler* routeRequest(const Poco::Net::HTTPServerRequest& request);
};

}  // namespace silo_api

#endif  // SILO_REQUEST_HANDLER_FACTORY_H
