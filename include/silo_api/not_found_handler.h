#ifndef SILO_ERROR_H
#define SILO_ERROR_H

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <nlohmann/json.hpp>

namespace silo_api {

class NotFoundHandler : public Poco::Net::HTTPRequestHandler {
   void handleRequest(
      Poco::Net::HTTPServerRequest& request,
      Poco::Net::HTTPServerResponse& response
   ) override;
};

}  // namespace silo_api

#endif  // SILO_ERROR_H
