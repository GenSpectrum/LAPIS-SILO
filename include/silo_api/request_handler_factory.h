#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

#include "silo_api/error_request_handler.h"

namespace silo_api {
class DatabaseMutex;
}  // namespace silo_api

namespace silo_api {

class SiloRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
  private:
   silo_api::DatabaseMutex& database;
   const StartupConfig startup_config;

  public:
   SiloRequestHandlerFactory(silo_api::DatabaseMutex& database, StartupConfig startup_config);

   Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);

  private:
   Poco::Net::HTTPRequestHandler* routeRequest(const Poco::Net::HTTPServerRequest& request);
};

}  // namespace silo_api
