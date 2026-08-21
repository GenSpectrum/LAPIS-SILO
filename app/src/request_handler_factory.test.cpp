#include <Poco/Net/HTTPResponse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "admin_query_handler.h"
#include "health_handler.h"
#include "info_handler.h"
#include "lineage_definition_handler.h"
#include "manual_poco_mocks.test.h"
#include "not_found_handler.h"
#include "query_handler.h"
#include "request_handler_factory.h"

using rhydb_app::RhyDBRequestHandlerFactory;

namespace {

std::unique_ptr<RhyDBRequestHandlerFactory> createRequestHandlerWithInitializedDatabase() {
   auto handle = std::make_shared<rhydb_app::ActiveDatabase>();
   auto table_schema = std::make_shared<rhydb::schema::TableSchema>();
   table_schema->primary_key = {.name = "primary_key", .type = rhydb::schema::ColumnType::STRING};
   table_schema->column_metadata.emplace(
      rhydb::schema::ColumnIdentifier{
         .name = "primary_key", .type = rhydb::schema::ColumnType::STRING
      },
      std::make_shared<rhydb::storage::column::StringColumnMetadata>("primary_key")
   );
   rhydb::schema::DatabaseSchema schema;
   schema.tables.emplace(rhydb::schema::TableName::getDefault(), table_schema);
   handle->setActiveDatabase(rhydb::Database(schema));
   auto request_handler = std::make_unique<RhyDBRequestHandlerFactory>(
      rhydb::config::RuntimeConfig::withDefaults(), handle
   );
   return request_handler;
}

template <typename HandlerType>
void assertHoldsHandlerType(std::unique_ptr<Poco::Net::HTTPRequestHandler>& handler) {
   EXPECT_NE(handler, nullptr);
   EXPECT_NE(dynamic_cast<HandlerType*>(handler.get()), nullptr);
}
}  // namespace

TEST(RhyDBRequestHandlerFactory, returns503ResponseWhenDatabaseIsNotInitializedOnInfoEndpoint) {
   rhydb_app::test::MockResponse response;
   rhydb_app::test::MockRequest request(response);
   request.setURI("/info");

   auto handle = std::make_shared<rhydb_app::ActiveDatabase>();
   RhyDBRequestHandlerFactory under_test{rhydb::config::RuntimeConfig::withDefaults(), handle};

   std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{under_test.createRequestHandler(request)};

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(RhyDBRequestHandlerFactory, returns503ResponseWhenDatabaseIsNotInitializedOnQueryEndpoint) {
   rhydb_app::test::MockResponse response;
   rhydb_app::test::MockRequest request(response);
   request.setMethod("POST");
   request.setURI("/query");

   auto handle = std::make_shared<rhydb_app::ActiveDatabase>();
   RhyDBRequestHandlerFactory under_test{rhydb::config::RuntimeConfig::withDefaults(), handle};

   std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{under_test.createRequestHandler(request)};

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(
   RhyDBRequestHandlerFactory,
   returns503ResponseWhenDatabaseIsNotInitializedOnLineageDefinitionEndpoint
) {
   rhydb_app::test::MockResponse response;
   rhydb_app::test::MockRequest request(response);
   request.setURI("/lineageDefinition/someColumn");

   auto handle = std::make_shared<rhydb_app::ActiveDatabase>();
   RhyDBRequestHandlerFactory under_test{rhydb::config::RuntimeConfig::withDefaults(), handle};

   std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{under_test.createRequestHandler(request)};

   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_SERVICE_UNAVAILABLE);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("Database not initialized yet"));
}

TEST(RhyDBRequestHandlerFactory, routesGetInfoRequest) {
   const Poco::URI uri("/info");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<rhydb_app::InfoHandler>(handler);
}

TEST(RhyDBRequestHandlerFactory, routesLineageDefinitionRequest) {
   const Poco::URI uri("/lineageDefinition/someId");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<rhydb_app::LineageDefinitionHandler>(handler);
}

TEST(RhyDBRequestHandlerFactory, routesPostQueryRequest) {
   const Poco::URI uri("/query");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<rhydb_app::QueryHandler>(handler);
}

TEST(RhyDBRequestHandlerFactory, routesPostAdminQueryRequest) {
   const Poco::URI uri("/admin/query");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<rhydb_app::AdminQueryHandler>(handler);
}

TEST(RhyDBRequestHandlerFactory, routesUnknownUrlToNotFoundHandler) {
   const Poco::URI uri("/unknown");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<rhydb_app::NotFoundHandler>(handler);
}

TEST(RhyDBRequestHandlerFactory, routesToHealth) {
   const Poco::URI uri("/health");

   auto under_test = createRequestHandlerWithInitializedDatabase();

   auto handler = under_test->routeRequest(uri);

   assertHoldsHandlerType<rhydb_app::HealthHandler>(handler);
}
