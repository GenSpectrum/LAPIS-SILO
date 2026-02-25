#include "silo/query_engine/saneql/ast_to_query.h"

#include <gtest/gtest.h>

#include "silo/query_engine/saneql/parse_exception.h"
#include "silo/query_engine/saneql/parser.h"

using silo::query_engine::saneql::ParseException;
using silo::query_engine::saneql::Parser;
using silo::query_engine::saneql::convertToQuery;

TEST(SaneQLAstToQuery, convertsSimpleFilterAndAggregated) {
   Parser parser("metadata.filter(country = 'USA').aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
   EXPECT_NE(query->action, nullptr);
}

TEST(SaneQLAstToQuery, convertsAndExpression) {
   Parser parser("metadata.filter(country = 'USA' && region = 'Europe').aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
}

TEST(SaneQLAstToQuery, convertsOrExpression) {
   Parser parser("metadata.filter(country = 'USA' || country = 'Germany').aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsNegation) {
   Parser parser("metadata.filter(!active).aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsIntEquals) {
   Parser parser("metadata.filter(age = 42).aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsBoolEquals) {
   Parser parser("metadata.filter(is_complete = true).aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsNullEquals) {
   Parser parser("metadata.filter(host = null).aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsAggregatedWithoutFilter) {
   Parser parser("metadata.aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
   EXPECT_NE(query->filter, nullptr);
   EXPECT_NE(query->action, nullptr);
}

TEST(SaneQLAstToQuery, convertsDetailsAction) {
   Parser parser("metadata.filter(country = 'USA').details(limit:=100)");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsStringInSet) {
   Parser parser("metadata.filter(country.in({'USA', 'Germany', 'France'})).aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, convertsIsNull) {
   Parser parser("metadata.filter(host.isNull()).aggregated()");
   auto ast = parser.parse();
   auto query = convertToQuery(*ast);
   ASSERT_NE(query, nullptr);
}

TEST(SaneQLAstToQuery, throwsOnMissingAction) {
   Parser parser("metadata.filter(country = 'USA')");
   auto ast = parser.parse();
   EXPECT_THROW(convertToQuery(*ast), ParseException);
}

TEST(SaneQLAstToQuery, throwsOnUnknownAction) {
   Parser parser("metadata.filter(country = 'USA').unknownAction()");
   auto ast = parser.parse();
   EXPECT_THROW(convertToQuery(*ast), ParseException);
}
