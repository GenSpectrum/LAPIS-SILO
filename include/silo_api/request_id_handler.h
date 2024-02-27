#pragma once

#include <memory>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>

namespace silo_api {

constexpr auto REQUEST_ID_HEADER = "X-Request-Id";

class RequestIdHandler : public Poco::Net::HTTPRequestHandler {
  private:
   std::unique_ptr<Poco::Net::HTTPRequestHandler> wrapped_handler;

  public:
   explicit RequestIdHandler(Poco::Net::HTTPRequestHandler* wrapped_handler);

   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;

  private:
   static std::string getRequestId(Poco::Net::HTTPServerRequest& request);
};
}  // namespace silo_api
