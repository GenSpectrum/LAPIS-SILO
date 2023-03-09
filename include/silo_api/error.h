#ifndef SILO_ERROR_H
#define SILO_ERROR_H

#include <nlohmann/json.hpp>
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"

namespace silo_api {
struct ErrorResponse {
   std::string error;
   std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResponse, error, message);

class NotFoundHandler : public Poco::Net::HTTPRequestHandler {
   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;
};
}  // namespace silo_api

#endif  // SILO_ERROR_H
