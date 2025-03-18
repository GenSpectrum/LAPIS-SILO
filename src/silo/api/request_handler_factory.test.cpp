#include <Poco/Net/HTTPResponse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/api/info_handler.h"
#include "silo/api/lineage_definition_handler.h"
#include "silo/api/manual_poco_mocks.test.h"
#include "silo/api/not_found_handler.h"
#include "silo/api/query_handler.h"
#include "silo/api/request_handler_factory.h"

using silo::api::SiloRequestHandlerFactory;

namespace {

std::unique_ptr<SiloRequestHandlerFactory> createRequestHandlerWithInitializedDatabase() {
   auto handle = std::make_shared<silo::api::ActiveDatabase>();
   handle->setActiveDatabase(silo::Database{silo::schema::DatabaseSchema::fromYAML(YAML::Load(R"(
default:
  primaryKey: primary_key
  columns:
    - name: primary_key
      type: string
      metadata:
         dictionary: []
)"))});
   auto request_handler = std::make_unique<SiloRequestHandlerFactory>(
      silo::config::RuntimeConfig::withDefaults(), handle
   );
   return request_handler;
}

template <typename HandlerType>
void assertHoldsHandlerType(std::unique_ptr<Poco::Net::HTTPRequestHandler>& handler) {
   EXPECT_NE(handler, nullptr);
   EXPECT_NE(dynamic_cast<HandlerType*>(handler.get()), nullptr);
}
}  // namespace

TEST(SiloRequestHandlerFactory, returns503ResponseWhenDatabaseIsNotInitializedOnInfoEndpoint) {
   silo::api::test::MockResponse response;
   silo::api::test::MockRequest request(response);
   request.setURI("/info");

   auto handle = std::make_shared<silo::api::ActiveDatabase>();
   SiloRequestHandlerFactory under_test{silo::config::RuntimeConfig::withDefaults(), handle};

   const auto handler = under_test.createRequestHandler(request);

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(SiloRequestHandlerFactory, returns503ResponseWhenDatabaseIsNotInitializedOnQueryEndpoint) {
   silo::api::test::MockResponse response;
   silo::api::test::MockRequest request(response);
   request.setMethod("POST");
   request.setURI("/query");

   auto handle = std::make_shared<silo::api::ActiveDatabase>();
   SiloRequestHandlerFactory under_test{silo::config::RuntimeConfig::withDefaults(), handle};

   const auto handler = under_test.createRequestHandler(request);

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(
   SiloRequestHandlerFactory,
   returns503ResponseWhenDatabaseIsNotInitializedOnLineageDefinitionEndpoint
) {
   silo::api::test::MockResponse response;
   silo::api::test::MockRequest request(response);
   request.setURI("/lineageDefinition/someColumn");

   auto handle = std::make_shared<silo::api::ActiveDatabase>();
   SiloRequestHandlerFactory under_test{silo::config::RuntimeConfig::withDefaults(), handle};

   const auto handler = under_test.createRequestHandler(request);

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(SiloRequestHandlerFactory, routesGetInfoRequest) {
   Poco::URI uri("/info");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo::api::InfoHandler>(handler);
}

TEST(SiloRequestHandlerFactory, routesLineageDefinitionRequest) {
   Poco::URI uri("/lineageDefinition/someId");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo::api::LineageDefinitionHandler>(handler);
}

TEST(SiloRequestHandlerFactory, routesPostQueryRequest) {
   Poco::URI uri("/query");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo::api::QueryHandler>(handler);
}

TEST(SiloRequestHandlerFactory, routesUnknownUrlToNotFoundHandler) {
   Poco::URI uri("/unknown");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo::api::NotFoundHandler>(handler);
}
