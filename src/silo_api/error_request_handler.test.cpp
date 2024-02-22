#include <Poco/Net/HTTPResponse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo_api/error_request_handler.h"
#include "silo_api/manual_poco_mocks.test.h"

class MockRequestHandler : public Poco::Net::HTTPRequestHandler {
  public:
   MOCK_METHOD(
      void,
      handleRequest,
      (Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse& response),
      ()
   );
};

const silo_api::StartupConfig TEST_STARTUP_CONFIG = {
   std::chrono::system_clock::now(),
   std::nullopt
};

TEST(ErrorRequestHandler, handlesRuntimeErrors) {
   auto* wrapped_handler_mock = new MockRequestHandler;

   auto under_test = silo_api::ErrorRequestHandler(wrapped_handler_mock, TEST_STARTUP_CONFIG);

   ON_CALL(*wrapped_handler_mock, handleRequest)
      .WillByDefault(testing::Throw(std::runtime_error("my error message")));

   silo_api::test::MockResponse response;
   silo_api::test::MockRequest request(response);
   under_test.handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
   EXPECT_EQ(
      response.out_stream.str(), R"({"error":"Internal Server Error","message":"my error message"})"
   );
}

TEST(ErrorRequestHandler, handlesOtherErrors) {
   auto* wrapped_handler_mock = new MockRequestHandler;

   auto under_test = silo_api::ErrorRequestHandler(wrapped_handler_mock, TEST_STARTUP_CONFIG);

   ON_CALL(*wrapped_handler_mock, handleRequest)
      .WillByDefault(testing::Throw(
         "One should not actually do this - since C++ admits it, throw a string here"
      ));

   silo_api::test::MockResponse response;
   silo_api::test::MockRequest request(response);
   under_test.handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
   EXPECT_EQ(response.out_stream.str(), R"({"error":"Internal Server Error","message":"PKc"})");
}

TEST(ErrorRequestHandler, doesNothingIfNoExceptionIsThrown) {
   const auto* wrapped_request_handler_message = "A message that the actual handler would write";
   auto* wrapped_handler_mock = new MockRequestHandler;

   auto under_test = silo_api::ErrorRequestHandler(wrapped_handler_mock, TEST_STARTUP_CONFIG);

   EXPECT_CALL(*wrapped_handler_mock, handleRequest).Times(testing::AtLeast(1));

   silo_api::test::MockResponse response;
   silo_api::test::MockRequest request(response);

   response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
   response.send() << wrapped_request_handler_message;

   under_test.handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_OK);
   EXPECT_EQ(response.out_stream.str(), wrapped_request_handler_message);
}
