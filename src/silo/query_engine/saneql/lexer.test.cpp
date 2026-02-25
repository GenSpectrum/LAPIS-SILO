#include "silo/query_engine/saneql/lexer.h"

#include <gtest/gtest.h>

#include "silo/query_engine/saneql/parse_exception.h"

using silo::query_engine::saneql::Lexer;
using silo::query_engine::saneql::ParseException;
using silo::query_engine::saneql::TokenType;

TEST(SaneQLLexer, tokenizesEmptyInput) {
   Lexer lexer("");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 1);
   EXPECT_EQ(tokens[0].type, TokenType::Eof);
}

TEST(SaneQLLexer, tokenizesIdentifier) {
   Lexer lexer("country");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 2);
   EXPECT_EQ(tokens[0].type, TokenType::Identifier);
   EXPECT_EQ(tokens[0].getStringValue(), "country");
   EXPECT_EQ(tokens[1].type, TokenType::Eof);
}

TEST(SaneQLLexer, tokenizesStringLiteral) {
   Lexer lexer("'hello world'");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 2);
   EXPECT_EQ(tokens[0].type, TokenType::StringLiteral);
   EXPECT_EQ(tokens[0].getStringValue(), "hello world");
}

TEST(SaneQLLexer, tokenizesIntLiteral) {
   Lexer lexer("42");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 2);
   EXPECT_EQ(tokens[0].type, TokenType::IntLiteral);
   EXPECT_EQ(tokens[0].getIntValue(), 42);
}

TEST(SaneQLLexer, tokenizesFloatLiteral) {
   Lexer lexer("3.14");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 2);
   EXPECT_EQ(tokens[0].type, TokenType::FloatLiteral);
   EXPECT_DOUBLE_EQ(tokens[0].getFloatValue(), 3.14);
}

TEST(SaneQLLexer, tokenizesBoolLiterals) {
   Lexer lexer("true false");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 3);
   EXPECT_EQ(tokens[0].type, TokenType::BoolLiteral);
   EXPECT_TRUE(tokens[0].getBoolValue());
   EXPECT_EQ(tokens[1].type, TokenType::BoolLiteral);
   EXPECT_FALSE(tokens[1].getBoolValue());
}

TEST(SaneQLLexer, tokenizesNullLiteral) {
   Lexer lexer("null");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 2);
   EXPECT_EQ(tokens[0].type, TokenType::NullLiteral);
}

TEST(SaneQLLexer, tokenizesOperators) {
   Lexer lexer("= <> < > <= >= && || !");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 10);
   EXPECT_EQ(tokens[0].type, TokenType::Equals);
   EXPECT_EQ(tokens[1].type, TokenType::NotEquals);
   EXPECT_EQ(tokens[2].type, TokenType::LessThan);
   EXPECT_EQ(tokens[3].type, TokenType::GreaterThan);
   EXPECT_EQ(tokens[4].type, TokenType::LessEqual);
   EXPECT_EQ(tokens[5].type, TokenType::GreaterEqual);
   EXPECT_EQ(tokens[6].type, TokenType::And);
   EXPECT_EQ(tokens[7].type, TokenType::Or);
   EXPECT_EQ(tokens[8].type, TokenType::Not);
}

TEST(SaneQLLexer, tokenizesPunctuation) {
   Lexer lexer(". , ( ) { } :: :=");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 9);
   EXPECT_EQ(tokens[0].type, TokenType::Dot);
   EXPECT_EQ(tokens[1].type, TokenType::Comma);
   EXPECT_EQ(tokens[2].type, TokenType::LeftParen);
   EXPECT_EQ(tokens[3].type, TokenType::RightParen);
   EXPECT_EQ(tokens[4].type, TokenType::LeftBrace);
   EXPECT_EQ(tokens[5].type, TokenType::RightBrace);
   EXPECT_EQ(tokens[6].type, TokenType::DoubleColon);
   EXPECT_EQ(tokens[7].type, TokenType::ColonEquals);
}

TEST(SaneQLLexer, tokenizesMethodCallChain) {
   Lexer lexer("metadata.filter(country = 'USA').aggregated()");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 13);
   EXPECT_EQ(tokens[0].type, TokenType::Identifier);
   EXPECT_EQ(tokens[0].getStringValue(), "metadata");
   EXPECT_EQ(tokens[1].type, TokenType::Dot);
   EXPECT_EQ(tokens[2].type, TokenType::Identifier);
   EXPECT_EQ(tokens[2].getStringValue(), "filter");
   EXPECT_EQ(tokens[3].type, TokenType::LeftParen);
   EXPECT_EQ(tokens[4].type, TokenType::Identifier);
   EXPECT_EQ(tokens[4].getStringValue(), "country");
   EXPECT_EQ(tokens[5].type, TokenType::Equals);
   EXPECT_EQ(tokens[6].type, TokenType::StringLiteral);
   EXPECT_EQ(tokens[6].getStringValue(), "USA");
   EXPECT_EQ(tokens[7].type, TokenType::RightParen);
   EXPECT_EQ(tokens[8].type, TokenType::Dot);
   EXPECT_EQ(tokens[9].type, TokenType::Identifier);
   EXPECT_EQ(tokens[9].getStringValue(), "aggregated");
   EXPECT_EQ(tokens[10].type, TokenType::LeftParen);
   EXPECT_EQ(tokens[11].type, TokenType::RightParen);
   EXPECT_EQ(tokens[12].type, TokenType::Eof);
}

TEST(SaneQLLexer, tokenizesNamedParameters) {
   Lexer lexer("hasMutation(position:=1000, sequenceName:='segment1')");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 11);
   EXPECT_EQ(tokens[0].type, TokenType::Identifier);
   EXPECT_EQ(tokens[1].type, TokenType::LeftParen);
   EXPECT_EQ(tokens[2].type, TokenType::Identifier);
   EXPECT_EQ(tokens[2].getStringValue(), "position");
   EXPECT_EQ(tokens[3].type, TokenType::ColonEquals);
   EXPECT_EQ(tokens[4].type, TokenType::IntLiteral);
   EXPECT_EQ(tokens[5].type, TokenType::Comma);
   EXPECT_EQ(tokens[6].type, TokenType::Identifier);
   EXPECT_EQ(tokens[7].type, TokenType::ColonEquals);
   EXPECT_EQ(tokens[8].type, TokenType::StringLiteral);
   EXPECT_EQ(tokens[9].type, TokenType::RightParen);
}

TEST(SaneQLLexer, tokenizesTypeCast) {
   Lexer lexer("'2020-01-01'::date");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 4);
   EXPECT_EQ(tokens[0].type, TokenType::StringLiteral);
   EXPECT_EQ(tokens[1].type, TokenType::DoubleColon);
   EXPECT_EQ(tokens[2].type, TokenType::Identifier);
   EXPECT_EQ(tokens[2].getStringValue(), "date");
}

TEST(SaneQLLexer, skipsWhitespaceAndNewlines) {
   Lexer lexer("  metadata\n  .filter(\n    country = 'USA'\n  )");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 9);
   EXPECT_EQ(tokens[0].type, TokenType::Identifier);
   EXPECT_EQ(tokens[1].type, TokenType::Dot);
}

TEST(SaneQLLexer, tracksLineAndColumn) {
   Lexer lexer("a\nb");
   auto tokens = lexer.tokenizeAll();
   EXPECT_EQ(tokens[0].location.line, 1);
   EXPECT_EQ(tokens[0].location.column, 1);
   EXPECT_EQ(tokens[1].location.line, 2);
   EXPECT_EQ(tokens[1].location.column, 1);
}

TEST(SaneQLLexer, throwsOnUnterminatedString) {
   Lexer lexer("'unterminated");
   EXPECT_THROW(lexer.nextToken(), ParseException);
}

TEST(SaneQLLexer, throwsOnInvalidCharacter) {
   Lexer lexer("@");
   EXPECT_THROW(lexer.nextToken(), ParseException);
}

TEST(SaneQLLexer, skipsLineComments) {
   Lexer lexer("a -- this is a comment\nb");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 3);
   EXPECT_EQ(tokens[0].type, TokenType::Identifier);
   EXPECT_EQ(tokens[0].getStringValue(), "a");
   EXPECT_EQ(tokens[1].type, TokenType::Identifier);
   EXPECT_EQ(tokens[1].getStringValue(), "b");
}

TEST(SaneQLLexer, tokenizesSetLiteral) {
   Lexer lexer("{'USA', 'Germany', 'France'}");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 8);
   EXPECT_EQ(tokens[0].type, TokenType::LeftBrace);
   EXPECT_EQ(tokens[1].type, TokenType::StringLiteral);
   EXPECT_EQ(tokens[1].getStringValue(), "USA");
   EXPECT_EQ(tokens[2].type, TokenType::Comma);
   EXPECT_EQ(tokens[3].type, TokenType::StringLiteral);
   EXPECT_EQ(tokens[4].type, TokenType::Comma);
   EXPECT_EQ(tokens[5].type, TokenType::StringLiteral);
   EXPECT_EQ(tokens[6].type, TokenType::RightBrace);
}

TEST(SaneQLLexer, handlesEscapedQuotesInStrings) {
   Lexer lexer("'it\\'s'");
   auto tokens = lexer.tokenizeAll();
   ASSERT_EQ(tokens.size(), 2);
   EXPECT_EQ(tokens[0].type, TokenType::StringLiteral);
   EXPECT_EQ(tokens[0].getStringValue(), "it's");
}
