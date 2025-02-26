#pragma once

#include <memory>

#include <crow.h>

namespace silo::api {

constexpr auto REQUEST_ID_HEADER = "X-Request-Id";

class RequestIdMiddleware {
  public:
   struct context {};

   void before_handle(crow::request& request, crow::response& response, context& context);

   void after_handle(crow::request& request, crow::response& response, context& context);
};
}  // namespace silo::api
