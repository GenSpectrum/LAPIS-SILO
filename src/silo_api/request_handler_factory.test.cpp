#include <Poco/Net/HTTPResponse.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/database_info.h"
#include "silo/query_engine/query_engine.h"
#include "silo/query_engine/query_result.h"
#include "silo_api/manual_poco_mocks.test.h"
#include "silo_api/request_handler_factory.h"

class MockDatabase : public silo::Database {
  public:
   MOCK_METHOD(silo::DatabaseInfo, getDatabaseInfo, (), (const));
   MOCK_METHOD(silo::DetailedDatabaseInfo, detailedDatabaseInfo, (), (const));
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

   void processRequest() {
      std::unique_ptr<Poco::Net::HTTPRequestHandler> request_handler(
         under_test.createRequestHandler(request)
      );
      request_handler->handleRequest(request, response);
   }
};

TEST_F(RequestHandlerTestFixture, handlesGetInfoRequest) {
   EXPECT_CALL(mock_database, getDatabaseInfo)
      .WillRepeatedly(testing::Return(silo::DatabaseInfo{1, 2, 3}));

   request.setURI("/info");

   processRequest();

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_OK);
   EXPECT_EQ(response.out_stream.str(), R"({"nBitmapsSize":3,"sequenceCount":1,"totalSize":2})");
}

TEST_F(RequestHandlerTestFixture, handlesGetInfoRequestDetails) {
   silo::BitmapSizePerSymbol bitmap_size_per_symbol;
   bitmap_size_per_symbol.size_in_bytes[silo::NUCLEOTIDE_SYMBOL::A] =
      1234;  // NOLINT(readability-magic-numbers)

   const silo::BitmapContainerSize bitmap_container_size(4567
   );  // NOLINT(readability-magic-numbers)

   const silo::DetailedDatabaseInfo detailed_database_info = {
      bitmap_size_per_symbol, bitmap_container_size};

   EXPECT_CALL(mock_database, detailedDatabaseInfo)
      .WillRepeatedly(testing::Return(detailed_database_info));

   request.setURI("/info?details=true");

   processRequest();

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_OK);
   EXPECT_EQ(
      response.out_stream.str(),
      R"({"bitmapContainerSizePerGenomeSection":{"bitmapContainerSizeStatistic":{"numberOfArrayContainers":0,"numberOfBitsetContainers":0,"numberOfRunContainers":0,"numberOfValuesStoredInArrayContainers":0,"numberOfValuesStoredInBitsetContainers":0,"numberOfValuesStoredInRunContainers":0,"totalBitmapSizeArrayContainers":0,"totalBitmapSizeBitsetContainers":0,"totalBitmapSizeRunContainers":0},"sectionLength":4567,"sizePerGenomeSymbolAndSection":{"-":[0,0,0,0,0,0,0],"N":[0,0,0,0,0,0,0],"NOT_N_NOT_GAP":[0,0,0,0,0,0,0]},"totalBitmapSizeComputed":0,"totalBitmapSizeFrozen":0},"bitmapSizePerSymbol":{"-":0,"A":1234,"B":0,"C":0,"D":0,"G":0,"H":0,"K":0,"M":0,"N":0,"R":0,"S":0,"T":0,"V":0,"W":0,"Y":0}})"
   );  // NOLINT
}

TEST_F(RequestHandlerTestFixture, returnsMethodNotAllowedOnPostInfoRequest) {
   request.setMethod("POST");
   request.setURI("/info");

   processRequest();

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
   EXPECT_EQ(
      response.out_stream.str(),
      R"({"error":"Method not allowed","message":"POST is not allowed on resource /info"})"
   );
}

static const silo::response::QueryResult QUERY_RESULT =
   silo::response::QueryResult{{silo::response::AggregationResult{5}}, 1, 2, 3};

TEST_F(RequestHandlerTestFixture, handlesPostQueryRequest) {
   EXPECT_CALL(mock_query_engine, executeQuery).WillRepeatedly(testing::Return(QUERY_RESULT));

   request.setMethod("POST");
   request.setURI("/query");

   processRequest();

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_OK);
   EXPECT_EQ(
      response.out_stream.str(),
      R"({"actionTime":3,"filterTime":2,"parseTime":1,"queryResult":{"count":5}})"
   );
}

TEST_F(RequestHandlerTestFixture, returnsMethodNotAllowedOnGetQuery) {
   request.setMethod("GET");
   request.setURI("/query");

   processRequest();

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
   EXPECT_EQ(
      response.out_stream.str(),
      R"({"error":"Method not allowed","message":"GET is not allowed on resource /query"})"
   );
}

TEST_F(RequestHandlerTestFixture, givenRequestToUnknownUrl_thenReturnsNotFound) {
   auto under_test = silo_api::SiloRequestHandlerFactory(mock_database, mock_query_engine);

   request.setURI("/doesNotExist");

   processRequest();

   EXPECT_EQ(response.getStatus(), Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
   EXPECT_EQ(
      response.out_stream.str(),
      R"({"error":"Not found","message":"Resource /doesNotExist does not exist"})"
   );
}
