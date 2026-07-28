#include <Poco/Net/HTTPResponse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "manual_poco_mocks.test.h"
#include "request_id_handler.h"

using silo_app::RequestIdHandler;

namespace {

class MockRequestHandler : public Poco::Net::HTTPRequestHandler {
  public:
   MOCK_METHOD(
      void,
      handleRequest,
      (Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse& response),
      ()
   );
};

}  // namespace

TEST(RequestIdHandler, givenNoRequestIdIsSet_thenGeneratesOne) {
   auto wrapped_handler_mock = std::make_unique<MockRequestHandler>();
   EXPECT_CALL(*wrapped_handler_mock, handleRequest);
   auto under_test = RequestIdHandler(std::move(wrapped_handler_mock));

   silo_app::test::MockResponse response;
   silo_app::test::MockRequest request(response);
   under_test.handleRequest(request, response);

   EXPECT_THAT(response.get("X-Request-Id"), ::testing::ContainsRegex("-[A-Za-z0-9]{4}-"));
}

TEST(RequestIdHandler, givenRequestIdIsSet_thenResponseAlsoContainsIt) {
   const std::string request_id_value = "request id value";

   auto wrapped_handler_mock = std::make_unique<MockRequestHandler>();
   EXPECT_CALL(*wrapped_handler_mock, handleRequest);
   auto under_test = RequestIdHandler(std::move(wrapped_handler_mock));

   silo_app::test::MockResponse response;
   silo_app::test::MockRequest request(response);
   request.set("X-Request-Id", request_id_value);
   under_test.handleRequest(request, response);

   EXPECT_EQ(response.get("X-Request-Id"), request_id_value);
}
