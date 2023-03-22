#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/database.h"
#include "silo/query_engine/query_engine.h"
#include "silo/query_engine/query_result.h"
#include "silo_api/manual_poco_mocks.test.h"
#include "silo_api/request_handler_factory.h"

class MockDatabase : public silo::Database {
  public:
   MOCK_METHOD(silo::DatabaseInfo, getDatabaseInfo, (), (const));
};

class MockQueryEngine : public silo::QueryEngine {
  public:
   explicit MockQueryEngine(const silo::Database& database)
       : QueryEngine(database) {}

   MOCK_METHOD(silo::response::QueryResult, executeQuery, (const std::string&), (const));
};

class RequestHandlerTestFixture : public ::testing::Test {
  protected:
   MockDatabase mock_database;
   MockQueryEngine mock_query_engine;
   silo_api::test::MockResponse response;
   silo_api::test::MockRequest request;
   silo_api::SiloRequestHandlerFactory under_test;

   RequestHandlerTestFixture()
       : mock_query_engine(MockQueryEngine(mock_database)),
         request(silo_api::test::MockRequest(response)),
         under_test(silo_api::SiloRequestHandlerFactory(mock_database, mock_query_engine)) {}
};

TEST_F(RequestHandlerTestFixture, handlesInfoRequest) {
   EXPECT_CALL(mock_database, getDatabaseInfo)
      .WillRepeatedly(testing::Return(silo::DatabaseInfo{1, 2, 3}));

   request.setURI("/info");

   under_test.createRequestHandler(request)->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), 200);
   EXPECT_EQ(response.out_stream.str(), R"({"nBitmapsSize":3,"sequenceCount":1,"totalSize":2})");
}

static const silo::response::QueryResult QUERY_RESULT =
   silo::response::QueryResult{silo::response::AggregationResult{5}, 1, 2, 3};

TEST_F(RequestHandlerTestFixture, handlesQueryRequest) {
   EXPECT_CALL(mock_query_engine, executeQuery).WillRepeatedly(testing::Return(QUERY_RESULT));

   request.setURI("/query");

   under_test.createRequestHandler(request)->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), 200);
   EXPECT_EQ(
      response.out_stream.str(),
      R"({"actionTime":3,"filterTime":2,"parseTime":1,"queryResult":{"count":5}})"
   );
}

TEST_F(RequestHandlerTestFixture, givenRequestToUnknownUrl_thenReturnsNotFound) {
   auto under_test = silo_api::SiloRequestHandlerFactory(mock_database, mock_query_engine);

   request.setURI("/doesNotExist");

   under_test.createRequestHandler(request)->handleRequest(request, response);

   EXPECT_EQ(response.getStatus(), 404);
   EXPECT_EQ(
      response.out_stream.str(), R"({"error":"Not found","message":"Resource does not exist"})"
   );
}
