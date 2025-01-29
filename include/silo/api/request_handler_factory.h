#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>

#include "database_mutex.h"
#include "error_request_handler.h"
#include "silo/config/runtime_config.h"

namespace silo::api {

class SiloRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
  private:
   silo::api::DatabaseMutex& database;
   const silo::config::RuntimeConfig runtime_config;

  public:
   SiloRequestHandlerFactory(
      silo::api::DatabaseMutex& database,
      silo::config::RuntimeConfig runtime_config
   );

   Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);

  private:
   std::unique_ptr<Poco::Net::HTTPRequestHandler> routeRequest(
      const Poco::Net::HTTPServerRequest& request
   );
};

}  // namespace silo::api
