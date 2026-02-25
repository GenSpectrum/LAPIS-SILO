#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "silo/query_engine/saneql/ast.h"
#include "silo/query_engine/saneql/lexer.h"
#include "silo/query_engine/saneql/token.h"

namespace silo::query_engine::saneql {

class Parser {
   Lexer lexer;
   Token current_token;

  public:
   explicit Parser(std::string_view input);

   [[nodiscard]] ast::ExpressionPtr parse();

  private:
   void advance();
   [[nodiscard]] const Token& current() const;
   Token expect(TokenType type);
   [[nodiscard]] bool check(TokenType type) const;
   bool match(TokenType type);

   [[nodiscard]] ast::ExpressionPtr parseExpression();
   [[nodiscard]] ast::ExpressionPtr parseOrExpr();
   [[nodiscard]] ast::ExpressionPtr parseAndExpr();
   [[nodiscard]] ast::ExpressionPtr parseNotExpr();
   [[nodiscard]] ast::ExpressionPtr parseComparisonExpr();
   [[nodiscard]] ast::ExpressionPtr parsePostfixExpr();
   [[nodiscard]] ast::ExpressionPtr parsePrimaryExpr();
   [[nodiscard]] ast::ExpressionPtr parseSetOrRecordExpression();
   [[nodiscard]] ast::ExpressionPtr parseIdentifierOrFunctionCall();
   [[nodiscard]] ast::ExpressionPtr parseLiteral();

   struct ParsedArgs {
      std::vector<ast::PositionalArgument> positional;
      std::vector<ast::NamedArgument> named;
   };
   [[nodiscard]] ParsedArgs parseArgList();
};

}  // namespace silo::query_engine::saneql
