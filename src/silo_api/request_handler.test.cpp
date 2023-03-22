#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/SocketAddress.h"

#include "silo_api/request_handler.h"

class MockRequestHandler : public Poco::Net::HTTPRequestHandler {
  public:
   MOCK_METHOD(
      void,
      handleRequest,
      (Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse& response),
      ()
   );
};

class MockResponse : public Poco::Net::HTTPServerResponse {
  public:
   std::stringstream out_stream;

   void sendContinue() override {}

   std::ostream& send() override { return out_stream; }

   void sendFile(const std::string& path, const std::string& mediaType) override {}

   void sendBuffer(const void* pBuffer, std::size_t length) override {}

   void redirect(const std::string& uri, HTTPStatus status = HTTP_FOUND) override {}

   void requireAuthentication(const std::string& realm) override {}

   bool sent() const override { return true; }
};

class MockRequest : public Poco::Net::HTTPServerRequest {
  public:
   std::stringstream in_stream;
   Poco::Net::SocketAddress address;
   Poco::Net::HTTPServerParams* params;
   MockResponse& mockResponse;

   explicit MockRequest(MockResponse& mockResponse)
       : mockResponse(mockResponse) {}

   ~MockRequest() override = default;

   std::istream& stream() override { return in_stream; }

   const Poco::Net::SocketAddress& clientAddress() const override { return address; }

   const Poco::Net::SocketAddress& serverAddress() const override { return address; }

   const Poco::Net::HTTPServerParams& serverParams() const override { return *params; }

   Poco::Net::HTTPServerResponse& response() const override { return mockResponse; }

   bool secure() const override { return false; }
};

TEST(ErrorRequestHandler, handlesRuntimeErrors) {
   auto* wrapped_handler_mock = new MockRequestHandler;

   auto under_test = silo_api::ErrorRequestHandler(wrapped_handler_mock);

   ON_CALL(*wrapped_handler_mock, handleRequest)
      .WillByDefault(testing::Throw(std::runtime_error("my error message")));

   MockResponse response;
   MockRequest request(response);
   under_test.handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), 500);
   EXPECT_EQ(
      response.out_stream.str(), R"({"error":"Internal server error","message":"my error message"})"
   );
}

TEST(ErrorRequestHandler, handlesOtherErrors) {
   auto* wrapped_handler_mock = new MockRequestHandler;

   auto under_test = silo_api::ErrorRequestHandler(wrapped_handler_mock);

   ON_CALL(*wrapped_handler_mock, handleRequest)
      .WillByDefault(testing::Throw(
         "One should not actually do this - since C++ admits it, throw a string here"
      ));

   MockResponse response;
   MockRequest request(response);
   under_test.handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), 500);
   EXPECT_EQ(
      response.out_stream.str(),
      R"({"error":"Internal server error","message":"Caught something: PKc"})"
   );
}

TEST(ErrorRequestHandler, does_nothing_if_not_exception_is_thrown) {
   const auto* wrapped_request_handler_message = "A message that the actual handler would write";
   auto* wrapped_handler_mock = new MockRequestHandler;

   auto under_test = silo_api::ErrorRequestHandler(wrapped_handler_mock);

   EXPECT_CALL(*wrapped_handler_mock, handleRequest).Times(testing::AtLeast(1));

   MockResponse response;
   MockRequest request(response);

   response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
   response.send() << wrapped_request_handler_message;

   under_test.handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), 200);
   EXPECT_EQ(response.out_stream.str(), wrapped_request_handler_message);
}
