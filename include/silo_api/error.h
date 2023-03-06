#ifndef SILO_ERROR_H
#define SILO_ERROR_H

#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include <nlohmann/json.hpp>

namespace silo_api {
struct ErrorResponse {
   std::string error;
   std::string message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ErrorResponse, error, message);

class NotFoundHandler : public Poco::Net::HTTPRequestHandler {
   void handleRequest(Poco::Net::HTTPServerRequest&, Poco::Net::HTTPServerResponse& response);
};
}

#endif //SILO_ERROR_H
