#include "silo/query_engine/saneql/parser.h"

#include <gmock/gmock.h>
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
   Parser parser("default.filter(country = 'USA')");
   auto expr = parser.parse();
   // Method call syntax is desugared: receiver becomes first positional arg
   EXPECT_EQ(expr->toString(), "filter(default, (country = 'USA'))");
}

TEST(SaneQLParser, parsesMethodCallChain) {
   Parser parser("default.filter(country = 'USA').groupBy({count:=count()})");
   auto expr = parser.parse();
   EXPECT_EQ(expr->toString(), "groupBy(filter(default, (country = 'USA')), {count:=count()})");
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
   EXPECT_EQ(expr->toString(), "in(country, {'USA', 'Germany'})");
}

TEST(SaneQLParser, parsesComplexFilterQuery) {
   Parser parser("default.filter(country = 'USA' && age > 30).groupBy({count:=count()})");
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::FunctionCall>(expr->value));
   auto& outer = std::get<ast::FunctionCall>(expr->value);
   EXPECT_EQ(outer.function_name, "groupBy");
   // First positional arg is the child pipeline (filter call)
   ASSERT_TRUE(std::holds_alternative<ast::FunctionCall>(outer.positional_arguments[0].value->value)
   );
   auto& filter = std::get<ast::FunctionCall>(outer.positional_arguments[0].value->value);
   EXPECT_EQ(filter.function_name, "filter");
}

TEST(SaneQLParser, parsesDateBetweenWithTypeCast) {
   Parser parser("date_submitted.between('2020-01-01'::date, '2023-12-31'::date)");
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::FunctionCall>(expr->value));
   auto& call = std::get<ast::FunctionCall>(expr->value);
   EXPECT_EQ(call.function_name, "between");
   // 3 positional arguments: receiver (date_submitted) + 2 date args
   ASSERT_EQ(call.positional_arguments.size(), 3);
}

TEST(SaneQLParser, parsesLimitMethod) {
   Parser parser("default.filter(country = 'USA').limit(100)");
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::FunctionCall>(expr->value));
   auto& limit_call = std::get<ast::FunctionCall>(expr->value);
   EXPECT_EQ(limit_call.function_name, "limit");
   // 2 positional arguments: receiver (filter(...)) + 100
   ASSERT_EQ(limit_call.positional_arguments.size(), 2);
}

TEST(SaneQLParser, parsesComparisonOperators) {
   Parser parser("age < 30");
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::BinaryExpr>(expr->value));
   auto& bin = std::get<ast::BinaryExpr>(expr->value);
   EXPECT_EQ(bin.op, ast::BinaryOp::LESS_THAN);
}

TEST(SaneQLParser, parsesFullExampleQuery) {
   Parser parser(
      "metadata\n"
      "  .filter(country = 'USA' && date_submitted.between('2020-01-01'::date, "
      "'2023-12-31'::date))\n"
      "  .groupBy({count:=count()})"
   );
   auto expr = parser.parse();
   ASSERT_TRUE(std::holds_alternative<ast::FunctionCall>(expr->value));
   auto& agg = std::get<ast::FunctionCall>(expr->value);
   EXPECT_EQ(agg.function_name, "groupBy");
}

TEST(SaneQLParser, throwsOnUnexpectedToken) {
   EXPECT_THAT(
      []() {
         Parser parser("= 'broken'");
         (void)parser.parse();
      },
      ThrowsMessage<ParseException>(
         ::testing::HasSubstr("arse error at 1:1: Unexpected token Equals")
      )
   );
}

TEST(SaneQLParser, throwsOnMissingClosingParen) {
   EXPECT_THAT(
      []() {
         Parser parser("func(a, b");
         (void)parser.parse();
      },
      ThrowsMessage<ParseException>(
         ::testing::HasSubstr("Parse error at 1:10: Expected RightParen but got Eof")
      )
   );
}

TEST(SaneQLParser, throwsOnTrailingGarbage) {
   EXPECT_THAT(
      []() {
         Parser parser("a b");
         (void)parser.parse();
      },
      ThrowsMessage<ParseException>(
         ::testing::HasSubstr("Parse error at 1:3: Expected Eof but got Identifier")
      )
   );
}
