#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/api/request_id_middleware.h"

using silo::api::RequestIdMiddleware;

TEST(RequestIdHandler, givenNoRequestIdIsSet_thenGeneratesOne) {
   crow::request request;
   crow::response response;
   RequestIdMiddleware::context context{};
   RequestIdMiddleware{}.before_handle(request, response, context);

   EXPECT_THAT(
      response.get_header_value("X-Request-Id"), ::testing::ContainsRegex("-[A-Za-z0-9]{4}-")
   );
}

TEST(RequestIdHandler, givenRequestIdIsSet_thenResponseAlsoContainsIt) {
   const std::string request_id_value = "request id value";

   crow::request request;
   crow::response response;
   RequestIdMiddleware::context context{};

   request.add_header("X-Request-Id", request_id_value);

   RequestIdMiddleware{}.before_handle(request, response, context);

   EXPECT_EQ(response.get_header_value("X-Request-Id"), request_id_value);
}
