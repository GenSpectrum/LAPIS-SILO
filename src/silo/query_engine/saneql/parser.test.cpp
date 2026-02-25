#include "silo/query_engine/saneql/parser.h"

#include <gtest/gtest.h>

#include "silo/query_engine/saneql/ast.h"
#include "silo/query_engine/saneql/parse_exception.h"

using silo::query_engine::saneql::ParseException;
using silo::query_engine::saneql::Parser;
namespace ast = silo::query_engine::saneql::ast;

TEST(SaneQLParser, parsesIdentifier) {
   Parser parser("country");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "country");
}

TEST(SaneQLParser, parsesStringLiteral) {
   Parser parser("'USA'");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "'USA'");
}

TEST(SaneQLParser, parsesIntLiteral) {
   Parser parser("42");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "42");
}

TEST(SaneQLParser, parsesFloatLiteral) {
   Parser parser("3.14");
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::FloatLiteral>(expr->value));
   EXPECT_DOUBLE_EQ(std::get<ast::FloatLiteral>(expr->value).value, 3.14);
}

TEST(SaneQLParser, parsesBoolLiteral) {
   Parser parser("true");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "true");
}

TEST(SaneQLParser, parsesNullLiteral) {
   Parser parser("null");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "null");
}

TEST(SaneQLParser, parsesEquality) {
   Parser parser("country = 'USA'");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "(country = 'USA')");
}

TEST(SaneQLParser, parsesNotEquals) {
   Parser parser("country <> 'USA'");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "(country <> 'USA')");
}

TEST(SaneQLParser, parsesAndExpression) {
   Parser parser("a = 1 && b = 2");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "((a = 1) && (b = 2))");
}

TEST(SaneQLParser, parsesOrExpression) {
   Parser parser("a = 1 || b = 2");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "((a = 1) || (b = 2))");
}

TEST(SaneQLParser, parsesNotExpression) {
   Parser parser("!active");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "(!active)");
}

TEST(SaneQLParser, parsesAndOrPrecedence) {
   Parser parser("a = 1 && b = 2 || c = 3");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "(((a = 1) && (b = 2)) || (c = 3))");
}

TEST(SaneQLParser, parsesParenthesizedExpression) {
   Parser parser("(a || b) && c");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "((a || b) && c)");
}

TEST(SaneQLParser, parsesFunctionCall) {
   Parser parser("hasMutation('A123T')");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "hasMutation('A123T')");
}

TEST(SaneQLParser, parsesFunctionCallWithNamedArgs) {
   Parser parser("hasMutation(position:=1000, sequenceName:='segment1')");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "hasMutation(position:=1000, sequenceName:='segment1')");
}

TEST(SaneQLParser, parsesMethodCall) {
   Parser parser("metadata.filter(country = 'USA')");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "metadata.filter((country = 'USA'))");
}

TEST(SaneQLParser, parsesMethodCallChain) {
   Parser parser("metadata.filter(country = 'USA').aggregated()");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "metadata.filter((country = 'USA')).aggregated()");
}

TEST(SaneQLParser, parsesTypeCast) {
   Parser parser("'2020-01-01'::date");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "'2020-01-01'::date");
}

TEST(SaneQLParser, parsesSetLiteral) {
   Parser parser("{'USA', 'Germany', 'France'}");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "{'USA', 'Germany', 'France'}");
}

TEST(SaneQLParser, parsesEmptySetLiteral) {
   Parser parser("{}");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "{}");
}

TEST(SaneQLParser, parsesMethodCallOnSetLiteral) {
   Parser parser("country.in({'USA', 'Germany'})");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "country.in({'USA', 'Germany'})");
}

TEST(SaneQLParser, parsesComplexFilterQuery) {
   Parser parser("metadata.filter(country = 'USA' && age > 30).aggregated()");
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::MethodCall>(expr->value));
   auto& outer = std::get<ast::MethodCall>(expr->value);
   EXPECT_EQ(outer.method_name, "aggregated");
   ASSERT_TRUE(std::holds_alternative<ast::MethodCall>(outer.receiver->value));
   auto& filter = std::get<ast::MethodCall>(outer.receiver->value);
   EXPECT_EQ(filter.method_name, "filter");
}

TEST(SaneQLParser, parsesDateBetweenWithTypeCast) {
   Parser parser(
      "date_submitted.between('2020-01-01'::date, '2023-12-31'::date)"
   );
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::MethodCall>(expr->value));
   auto& call = std::get<ast::MethodCall>(expr->value);
   EXPECT_EQ(call.method_name, "between");
   ASSERT_EQ(call.arguments.size(), 2);
}

TEST(SaneQLParser, parsesDetailsWithNamedLimit) {
   Parser parser("metadata.filter(country = 'USA').details(limit:=100)");
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::MethodCall>(expr->value));
   auto& details = std::get<ast::MethodCall>(expr->value);
   EXPECT_EQ(details.method_name, "details");
   ASSERT_EQ(details.arguments.size(), 1);
   EXPECT_EQ(details.arguments[0].name.value(), "limit");
}

TEST(SaneQLParser, parsesComparisonOperators) {
   Parser parser("age < 30");
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::BinaryExpr>(expr->value));
   auto& bin = std::get<ast::BinaryExpr>(expr->value);
   EXPECT_EQ(bin.op, ast::BinaryOp::LessThan);
}

TEST(SaneQLParser, parsesFullExampleQuery) {
   Parser parser(
      "metadata\n"
      "  .filter(country = 'USA' && date_submitted.between('2020-01-01'::date, "
      "'2023-12-31'::date))\n"
      "  .aggregated()"
   );
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::MethodCall>(expr->value));
   auto& agg = std::get<ast::MethodCall>(expr->value);
   EXPECT_EQ(agg.method_name, "aggregated");
}

TEST(SaneQLParser, throwsOnUnexpectedToken) {
   Parser parser("= 'broken'");
   EXPECT_THROW(parser.parse(), ParseException);
}

TEST(SaneQLParser, throwsOnMissingClosingParen) {
   Parser parser("func(a, b");
   EXPECT_THROW(parser.parse(), ParseException);
}

TEST(SaneQLParser, throwsOnTrailingGarbage) {
   Parser parser("a b");
   EXPECT_THROW(parser.parse(), ParseException);
}
