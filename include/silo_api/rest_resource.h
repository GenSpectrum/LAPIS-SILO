#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

namespace silo_api {

class RestResource : public Poco::Net::HTTPRequestHandler {
   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;

   [[maybe_unused]] virtual void get(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   );

   [[maybe_unused]] virtual void post(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   );
};

};  // namespace silo_api
