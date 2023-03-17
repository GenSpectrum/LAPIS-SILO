#include "silo_api/request_handler.h"

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <spdlog/spdlog.h>

namespace silo_api {

LoggingRequestHandler::LoggingRequestHandler(Poco::Net::HTTPRequestHandler* wrapped_handler)
    : wrapped_handler(wrapped_handler){};

void LoggingRequestHandler::handleRequest(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   SPDLOG_INFO("Handling {} {}", request.getMethod(), request.getURI());

   wrapped_handler->handleRequest(request, response);

   SPDLOG_INFO("Responding with status code {}", response.getStatus());
}

}  // namespace silo_api
