#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/URI.h>

#include "active_database.h"
#include "error_request_handler.h"
#include "silo/config/runtime_config.h"

namespace silo::api {

class SiloRequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
  private:
   const silo::config::RuntimeConfig runtime_config;
   std::shared_ptr<ActiveDatabase> database_handle;

  public:
   SiloRequestHandlerFactory(
      silo::config::RuntimeConfig runtime_config,
      std::shared_ptr<ActiveDatabase> database_handle
   );

   Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request
   ) override;

   std::unique_ptr<Poco::Net::HTTPRequestHandler> routeRequest(const Poco::URI& uri);
};

}  // namespace silo::api
