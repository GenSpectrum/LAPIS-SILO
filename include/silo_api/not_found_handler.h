#pragma once

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
