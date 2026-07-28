#include <Poco/Net/HTTPResponse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "health_handler.h"
#include "info_handler.h"
#include "lineage_definition_handler.h"
#include "manual_poco_mocks.test.h"
#include "not_found_handler.h"
#include "query_handler.h"
#include "request_handler_factory.h"

using silo_app::SiloRequestHandlerFactory;

namespace {

std::unique_ptr<SiloRequestHandlerFactory> createRequestHandlerWithInitializedDatabase() {
   auto handle = std::make_shared<silo_app::ActiveDatabase>();
   auto table_schema = std::make_shared<silo::schema::TableSchema>();
   table_schema->primary_key = {.name = "primary_key", .type = silo::schema::ColumnType::STRING};
   table_schema->column_metadata.emplace(
      silo::schema::ColumnIdentifier{
         .name = "primary_key", .type = silo::schema::ColumnType::STRING
      },
      std::make_shared<silo::storage::column::StringColumnMetadata>("primary_key")
   );
   silo::schema::DatabaseSchema schema;
   schema.tables.emplace(silo::schema::TableName::getDefault(), table_schema);
   handle->setActiveDatabase(silo::Database(schema));
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
   silo_app::test::MockResponse response;
   silo_app::test::MockRequest request(response);
   request.setURI("/info");

   auto handle = std::make_shared<silo_app::ActiveDatabase>();
   SiloRequestHandlerFactory under_test{silo::config::RuntimeConfig::withDefaults(), handle};

   std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{under_test.createRequestHandler(request)};

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(SiloRequestHandlerFactory, returns503ResponseWhenDatabaseIsNotInitializedOnQueryEndpoint) {
   silo_app::test::MockResponse response;
   silo_app::test::MockRequest request(response);
   request.setMethod("POST");
   request.setURI("/query");

   auto handle = std::make_shared<silo_app::ActiveDatabase>();
   SiloRequestHandlerFactory under_test{silo::config::RuntimeConfig::withDefaults(), handle};

   std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{under_test.createRequestHandler(request)};

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(
   SiloRequestHandlerFactory,
   returns503ResponseWhenDatabaseIsNotInitializedOnLineageDefinitionEndpoint
) {
   silo_app::test::MockResponse response;
   silo_app::test::MockRequest request(response);
   request.setURI("/lineageDefinition/someColumn");

   auto handle = std::make_shared<silo_app::ActiveDatabase>();
   SiloRequestHandlerFactory under_test{silo::config::RuntimeConfig::withDefaults(), handle};

   std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{under_test.createRequestHandler(request)};

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(SiloRequestHandlerFactory, routesGetInfoRequest) {
   const Poco::URI uri("/info");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo_app::InfoHandler>(handler);
}

TEST(SiloRequestHandlerFactory, routesLineageDefinitionRequest) {
   const Poco::URI uri("/lineageDefinition/someId");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo_app::LineageDefinitionHandler>(handler);
}

TEST(SiloRequestHandlerFactory, routesPostQueryRequest) {
   const Poco::URI uri("/query");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo_app::QueryHandler>(handler);
}

TEST(SiloRequestHandlerFactory, routesUnknownUrlToNotFoundHandler) {
   const Poco::URI uri("/unknown");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo_app::NotFoundHandler>(handler);
}

TEST(SiloRequestHandlerFactory, routesToHealth) {
   const Poco::URI uri("/health");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<silo_app::HealthHandler>(handler);
}
