#pragma once

#include <crow.h>

namespace silo::api {

class LoggingRequestMiddleware {
  public:
  public:
   struct context {};

   void before_handle(crow::request& request, crow::response& response, context& context);

   void after_handle(crow::request& request, crow::response& response, context& context);
};
}  // namespace silo::api
