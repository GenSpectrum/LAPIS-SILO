#include "silo_api/logging_request_handler.h"

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

   SPDLOG_INFO("Responding with status code {}", static_cast<uint32_t>(response.getStatus()));
}

}  // namespace silo_api