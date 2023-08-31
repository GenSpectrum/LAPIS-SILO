#pragma once

#include <memory>
#include <string>

#include <Poco/Net/HTTPRequestHandler.h>
#include <nlohmann/json.hpp>

namespace silo_api {

struct ErrorResponse {
   std::string error;
   std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResponse, error, message);

class ErrorRequestHandler : public Poco::Net::HTTPRequestHandler {
  private:
   std::unique_ptr<Poco::Net::HTTPRequestHandler> wrapped_handler;

  public:
   explicit ErrorRequestHandler(Poco::Net::HTTPRequestHandler* wrapped_handler);

   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;
};

};  // namespace silo_api
