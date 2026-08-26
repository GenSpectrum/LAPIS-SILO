#include "admin_query_handler.h"

#include <map>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

#include <Poco/Net/HTTPResponse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <rhydb/config/runtime_config.h>
#include <rhydb/database.h>
#include <rhydb/schema/database_schema.h>
#include <rhydb/storage/column/string_column.h>

#include "active_database.h"
#include "manual_poco_mocks.test.h"
#include "request_handler_factory.h"

namespace {

using rhydb::schema::ColumnIdentifier;
using rhydb::schema::ColumnType;
using rhydb::schema::TableName;
using rhydb::schema::TableSchema;
using rhydb::storage::column::ColumnMetadata;
using rhydb::storage::column::StringColumnMetadata;

// A metadata-only schema (string primary key + a string and an int32 column) shared by the source
// and target tables of the test fixture.
std::shared_ptr<TableSchema> makeValueSchema() {
   const ColumnIdentifier key{.name = "key", .type = ColumnType::STRING};
   const ColumnIdentifier country{.name = "country", .type = ColumnType::STRING};
   const ColumnIdentifier age{.name = "age", .type = ColumnType::INT32};
   std::map<ColumnIdentifier, std::shared_ptr<ColumnMetadata>> column_metadata{
      {key, std::make_shared<StringColumnMetadata>(key.name)},
      {country, std::make_shared<StringColumnMetadata>(country.name)},
      {age, std::make_shared<ColumnMetadata>(age.name)},
   };
   return std::make_shared<TableSchema>(std::move(column_metadata), key);
}

// Active database holding a `source` table with three rows (two country='CH', one 'US') and an
// empty `archive` table with the same schema.
std::shared_ptr<rhydb_app::ActiveDatabase> makeActiveDatabaseWithSourceData() {
   rhydb::Database database;
   database.createTable(TableName{"source"}, makeValueSchema());
   database.createTable(TableName{"archive"}, makeValueSchema());

   std::stringstream source_data;
   source_data << R"({"key":"a","country":"CH","age":1})" << "\n"
               << R"({"key":"b","country":"US","age":2})" << "\n"
               << R"({"key":"c","country":"CH","age":3})" << "\n";
   database.appendData(TableName{"source"}, source_data);

   auto handle = std::make_shared<rhydb_app::ActiveDatabase>();
   handle->setActiveDatabase(std::move(database));
   return handle;
}

// The runtime config the API is served with in these tests: the write-enabled admin endpoint is
// opt-in, so it has to be switched on explicitly.
rhydb::config::RuntimeConfig configWithAdminEndpoint(bool allow_admin_endpoint) {
   auto runtime_config = rhydb::config::RuntimeConfig::withDefaults();
   runtime_config.api_options.allow_admin_endpoint = allow_admin_endpoint;
   return runtime_config;
}

// Routes `POST /admin/query` with the given body through the full handler factory (so request-id
// assignment and exception-to-status mapping are exercised) and returns the populated response.
void postAdminQuery(
   const std::shared_ptr<rhydb_app::ActiveDatabase>& handle,
   const std::string& query,
   rhydb_app::test::MockResponse& response,
   bool allow_admin_endpoint = true
) {
   rhydb_app::test::MockRequest request(response);
   request.setMethod("POST");
   request.setURI("/admin/query");
   request.in_stream << query;

   rhydb_app::RhyDBRequestHandlerFactory factory{
      configWithAdminEndpoint(allow_admin_endpoint), handle
   };
   std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{factory.createRequestHandler(request)};
   handler->handleRequest(request, response);
}

}  // namespace

TEST(AdminQueryHandler, insertsQueryResultAndReportsRowCount) {
   auto handle = makeActiveDatabaseWithSourceData();

   rhydb_app::test::MockResponse response;
   postAdminQuery(handle, "source.filter(country='CH').insertInto(archive)", response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_OK);

   const auto body = nlohmann::json::parse(response.out_stream.str());
   EXPECT_EQ(body.at("insertedRows").get<size_t>(), 2);

   // A successful write reports the data version of the database as it is after the write, so that
   // a client can tell that the data it queries next is the mutated data.
   ASSERT_TRUE(response.has("data-version"));
   EXPECT_THAT(response.get("data-version"), testing::MatchesRegex("[0-9]{10}"));
   EXPECT_EQ(
      response.get("data-version"), handle->getActiveDatabase()->getDataVersionTimestamp().value
   );

   // The rows really landed in the target table.
   EXPECT_EQ(
      handle->getActiveDatabase()->tables.at(TableName{"archive"})->row_layout.numRows(), 2U
   );
   // The source table is untouched.
   EXPECT_EQ(handle->getActiveDatabase()->tables.at(TableName{"source"})->row_layout.numRows(), 3U);
}

TEST(AdminQueryHandler, rejectsPlainReadQueryWithBadRequest) {
   auto handle = makeActiveDatabaseWithSourceData();

   rhydb_app::test::MockResponse response;
   postAdminQuery(handle, "source.filter(country='CH')", response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("expected a write statement"));
   // Nothing was inserted.
   EXPECT_EQ(
      handle->getActiveDatabase()->tables.at(TableName{"archive"})->row_layout.numRows(), 0U
   );
}

TEST(AdminQueryHandler, rejectsUnknownTargetTableWithBadRequest) {
   auto handle = makeActiveDatabaseWithSourceData();

   rhydb_app::test::MockResponse response;
   postAdminQuery(handle, "source.insertInto(does_not_exist)", response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
   EXPECT_THAT(
      response.out_stream.str(), testing::HasSubstr("target table 'does_not_exist' not found")
   );
}

// The admin endpoint is opt-in: with `api.allowAdminEndpoint` left at its default the path is not
// routed at all, so an instance that does not enable it stays read-only.
TEST(AdminQueryHandler, isNotServedWhenNotEnabled) {
   auto handle = makeActiveDatabaseWithSourceData();

   rhydb_app::test::MockResponse response;
   postAdminQuery(
      handle,
      "source.filter(country='CH').insertInto(archive)",
      response,
      /*allow_admin_endpoint=*/false
   );

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
   EXPECT_EQ(
      handle->getActiveDatabase()->tables.at(TableName{"archive"})->row_layout.numRows(), 0U
   );
}

// Two admin requests must never mutate the database at the same time. All requests go through one
// factory here (as they do in the server), so they share the write mutex; without it the concurrent
// inserts race on the target table's columns.
TEST(AdminQueryHandler, serializesConcurrentWrites) {
   auto handle = makeActiveDatabaseWithSourceData();

   rhydb_app::RhyDBRequestHandlerFactory factory{configWithAdminEndpoint(true), handle};

   constexpr size_t NUMBER_OF_THREADS = 4;
   constexpr size_t REQUESTS_PER_THREAD = 3;

   std::vector<std::thread> writers;
   writers.reserve(NUMBER_OF_THREADS);
   for (size_t thread_index = 0; thread_index < NUMBER_OF_THREADS; ++thread_index) {
      writers.emplace_back([&factory]() {
         for (size_t request_index = 0; request_index < REQUESTS_PER_THREAD; ++request_index) {
            rhydb_app::test::MockResponse response;
            rhydb_app::test::MockRequest request(response);
            request.setMethod("POST");
            request.setURI("/admin/query");
            request.in_stream << "source.filter(country='CH').insertInto(archive)";

            std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{
               factory.createRequestHandler(request)
            };
            handler->handleRequest(request, response);

            EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_OK);
         }
      });
   }
   for (auto& writer : writers) {
      writer.join();
   }

   // Every request inserted the two matching rows, none of them lost to a race.
   EXPECT_EQ(
      handle->getActiveDatabase()->tables.at(TableName{"archive"})->row_layout.numRows(),
      NUMBER_OF_THREADS * REQUESTS_PER_THREAD * 2
   );
}

// The read-only `/query` endpoint must reject a write (insert) statement (it parses to a
// WriteCommand, not a QueryNode), respond 400, and leave the target table untouched.
TEST(QueryHandler, rejectsInsertQueryOnReadOnlyEndpointWithBadRequest) {
   auto handle = makeActiveDatabaseWithSourceData();

   rhydb_app::test::MockResponse response;
   rhydb_app::test::MockRequest request(response);
   request.setMethod("POST");
   request.setURI("/query");
   request.in_stream << "source.filter(country='CH').insertInto(archive)";

   rhydb_app::RhyDBRequestHandlerFactory factory{configWithAdminEndpoint(true), handle};
   std::unique_ptr<Poco::Net::HTTPRequestHandler> handler{factory.createRequestHandler(request)};
   handler->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
   EXPECT_THAT(response.out_stream.str(), testing::HasSubstr("POST /admin/query"));
   // The guard fires before planning, so nothing is written.
   EXPECT_EQ(
      handle->getActiveDatabase()->tables.at(TableName{"archive"})->row_layout.numRows(), 0U
   );
}
