#include "silo/api/logging_request_middleware.h"

#include <spdlog/spdlog.h>

namespace silo::api {

void LoggingRequestMiddleware::before_handle(
   crow::request& request,
   crow::response& response,
   silo::api::LoggingRequestMiddleware::context& context
) {
   const auto request_id = response.get_header_value("X-Request-Id");
   SPDLOG_INFO(
      "Request Id [{}] - Handling {} {}", request_id, crow::method_name(request.method), request.url
   );
}

void LoggingRequestMiddleware::after_handle(
   crow::request& request,
   crow::response& response,
   silo::api::LoggingRequestMiddleware::context& context
) {
   const auto request_id = response.get_header_value("X-Request-Id");
   SPDLOG_INFO(
      "Request Id [{}] - Responding with status code {}",
      request_id,
      static_cast<uint32_t>(response.code)
   );
}

}  // namespace silo::api