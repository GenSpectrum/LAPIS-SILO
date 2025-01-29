#include "silo/api/logging_request_handler.h"

#include <spdlog/spdlog.h>

namespace silo::api {

LoggingRequestHandler::LoggingRequestHandler(Poco::Net::HTTPRequestHandler* wrapped_handler)
    : wrapped_handler(wrapped_handler) {}

void LoggingRequestHandler::handleRequest(
   Poco::Net::HTTPServerRequest& request,
   Poco::Net::HTTPServerResponse& response
) {
   const auto request_id = response.get("X-Request-Id");
   SPDLOG_INFO(
      "Request Id [{}] - Handling {} {}", request_id, request.getMethod(), request.getURI()
   );

   wrapped_handler->handleRequest(request, response);

   SPDLOG_INFO(
      "Request Id [{}] - Responding with status code {}",
      request_id,
      static_cast<uint32_t>(response.getStatus())
   );
}

}  // namespace silo::api