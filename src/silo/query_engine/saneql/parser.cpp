#include "silo/query_engine/saneql/parser.h"

#include "silo/query_engine/saneql/parse_exception.h"

namespace silo::query_engine::saneql {

Parser::Parser(std::string_view input)
    : lexer(input),
      current_token(lexer.nextToken()) {}

void Parser::advance() {
   current_token = lexer.nextToken();
}

const Token& Parser::current() const {
   return current_token;
}

Token Parser::expect(TokenType type) {
   if (current_token.type != type) {
      throw ParseException(
         current_token.location,
         "Expected {} but got {}",
         tokenTypeToString(type),
         tokenTypeToString(current_token.type)
      );
   }
   Token token = current_token;
   advance();
   return token;
}

bool Parser::check(TokenType type) const {
   return current_token.type == type;
}

bool Parser::match(TokenType type) {
   if (check(type)) {
      advance();
      return true;
   }
   return false;
}

ast::ExpressionPtr Parser::parse() {
   auto expr = parseExpression();
   expect(TokenType::Eof);
   return expr;
}

ast::ExpressionPtr Parser::parseExpression() {
   return parseOrExpr();
}

ast::ExpressionPtr Parser::parseOrExpr() {
   auto left = parseAndExpr();

   while (check(TokenType::Or)) {
      SourceLocation loc = current().location;
      advance();
      auto right = parseAndExpr();
      left =
         ast::makeExpr(ast::BinaryExpr{ast::BinaryOp::Or, std::move(left), std::move(right)}, loc);
   }

   return left;
}

ast::ExpressionPtr Parser::parseAndExpr() {
   auto left = parseNotExpr();

   while (check(TokenType::And)) {
      SourceLocation loc = current().location;
      advance();
      auto right = parseNotExpr();
      left =
         ast::makeExpr(ast::BinaryExpr{ast::BinaryOp::And, std::move(left), std::move(right)}, loc);
   }

   return left;
}

ast::ExpressionPtr Parser::parseNotExpr() {
   if (check(TokenType::Not)) {
      SourceLocation loc = current().location;
      advance();
      auto operand = parseNotExpr();
      return ast::makeExpr(ast::UnaryNotExpr{std::move(operand)}, loc);
   }
   return parseComparisonExpr();
}

ast::ExpressionPtr Parser::parseComparisonExpr() {
   auto left = parsePostfixExpr();

   if (check(TokenType::Equals) || check(TokenType::NotEquals) || check(TokenType::LessThan) ||
       check(TokenType::LessEqual) || check(TokenType::GreaterThan) ||
       check(TokenType::GreaterEqual)) {
      SourceLocation loc = current().location;
      ast::BinaryOp op;
      switch (current().type) {
         case TokenType::Equals:
            op = ast::BinaryOp::Equals;
            break;
         case TokenType::NotEquals:
            op = ast::BinaryOp::NotEquals;
            break;
         case TokenType::LessThan:
            op = ast::BinaryOp::LessThan;
            break;
         case TokenType::LessEqual:
            op = ast::BinaryOp::LessEqual;
            break;
         case TokenType::GreaterThan:
            op = ast::BinaryOp::GreaterThan;
            break;
         case TokenType::GreaterEqual:
            op = ast::BinaryOp::GreaterEqual;
            break;
         default:
            break;  // unreachable
      }
      advance();
      auto right = parsePostfixExpr();
      left = ast::makeExpr(ast::BinaryExpr{op, std::move(left), std::move(right)}, loc);
   }

   return left;
}

ast::ExpressionPtr Parser::parsePostfixExpr() {
   auto expr = parsePrimaryExpr();

   while (true) {
      if (check(TokenType::Dot)) {
         advance();
         Token method_name = expect(TokenType::Identifier);
         if (check(TokenType::LeftParen)) {
            advance();
            std::vector<ast::Argument> args;
            if (!check(TokenType::RightParen)) {
               args = parseArgList();
            }
            expect(TokenType::RightParen);
            expr = ast::makeExpr(
               ast::MethodCall{std::move(expr), method_name.getStringValue(), std::move(args)},
               method_name.location
            );
         } else {
            // Property access — treat as method call with no args for simplicity
            expr = ast::makeExpr(
               ast::MethodCall{std::move(expr), method_name.getStringValue(), {}},
               method_name.location
            );
         }
      } else if (check(TokenType::DoubleColon)) {
         SourceLocation loc = current().location;
         advance();
         Token type_name = expect(TokenType::Identifier);
         expr = ast::makeExpr(ast::TypeCast{std::move(expr), type_name.getStringValue()}, loc);
      } else {
         break;
      }
   }

   return expr;
}

ast::ExpressionPtr Parser::parsePrimaryExpr() {
   SourceLocation loc = current().location;

   if (check(TokenType::LeftParen)) {
      advance();
      auto expr = parseExpression();
      expect(TokenType::RightParen);
      return expr;
   }

   if (check(TokenType::LeftBrace)) {
      advance();
      std::vector<ast::ExpressionPtr> elements;
      if (!check(TokenType::RightBrace)) {
         elements.push_back(parseExpression());
         while (match(TokenType::Comma)) {
            elements.push_back(parseExpression());
         }
      }
      expect(TokenType::RightBrace);
      return ast::makeExpr(ast::SetLiteral{std::move(elements)}, loc);
   }

   if (check(TokenType::Identifier)) {
      std::string name = current().getStringValue();
      advance();

      if (check(TokenType::LeftParen)) {
         advance();
         std::vector<ast::Argument> args;
         if (!check(TokenType::RightParen)) {
            args = parseArgList();
         }
         expect(TokenType::RightParen);
         return ast::makeExpr(ast::FunctionCall{std::move(name), std::move(args)}, loc);
      }

      return ast::makeExpr(ast::Identifier{std::move(name)}, loc);
   }

   if (check(TokenType::StringLiteral)) {
      std::string val = current().getStringValue();
      advance();
      return ast::makeExpr(ast::StringLiteral{std::move(val)}, loc);
   }

   if (check(TokenType::IntLiteral)) {
      int64_t val = current().getIntValue();
      advance();
      return ast::makeExpr(ast::IntLiteral{val}, loc);
   }

   if (check(TokenType::FloatLiteral)) {
      double val = current().getFloatValue();
      advance();
      return ast::makeExpr(ast::FloatLiteral{val}, loc);
   }

   if (check(TokenType::BoolLiteral)) {
      bool val = current().getBoolValue();
      advance();
      return ast::makeExpr(ast::BoolLiteral{val}, loc);
   }

   if (check(TokenType::NullLiteral)) {
      advance();
      return ast::makeExpr(ast::NullLiteral{}, loc);
   }

   throw ParseException(loc, "Unexpected token {}", tokenTypeToString(current().type));
}

std::vector<ast::Argument> Parser::parseArgList() {
   std::vector<ast::Argument> args;
   args.push_back(parseArgument());
   while (match(TokenType::Comma)) {
      args.push_back(parseArgument());
   }
   return args;
}

ast::Argument Parser::parseArgument() {
   SourceLocation loc = current().location;

   // Check for named argument: IDENT ':=' expression
   if (check(TokenType::Identifier)) {
      // We need to peek ahead to see if this is name:=value
      // Save state by trying to parse as named
      std::string name = current().getStringValue();
      Token saved = current_token;

      advance();
      if (check(TokenType::ColonEquals)) {
         advance();
         auto value = parseExpression();
         return ast::Argument{name, std::move(value), loc};
      }

      // Not a named argument — put back by re-parsing from the identifier
      // We already consumed the identifier, so we need to build the expression
      // starting from it
      ast::ExpressionPtr expr;
      if (check(TokenType::LeftParen)) {
         // It was a function call
         advance();
         std::vector<ast::Argument> args;
         if (!check(TokenType::RightParen)) {
            args = parseArgList();
         }
         expect(TokenType::RightParen);
         expr = ast::makeExpr(ast::FunctionCall{std::move(name), std::move(args)}, saved.location);
      } else {
         expr = ast::makeExpr(ast::Identifier{std::move(name)}, saved.location);
      }

      // Continue parsing the rest of the expression (postfix, comparison, etc.)
      // We need to handle postfix operations on the identifier
      while (true) {
         if (check(TokenType::Dot)) {
            advance();
            Token method_name = expect(TokenType::Identifier);
            if (check(TokenType::LeftParen)) {
               advance();
               std::vector<ast::Argument> method_args;
               if (!check(TokenType::RightParen)) {
                  method_args = parseArgList();
               }
               expect(TokenType::RightParen);
               expr = ast::makeExpr(
                  ast::MethodCall{
                     std::move(expr), method_name.getStringValue(), std::move(method_args)
                  },
                  method_name.location
               );
            } else {
               expr = ast::makeExpr(
                  ast::MethodCall{std::move(expr), method_name.getStringValue(), {}},
                  method_name.location
               );
            }
         } else if (check(TokenType::DoubleColon)) {
            SourceLocation cast_loc = current().location;
            advance();
            Token type_name = expect(TokenType::Identifier);
            expr =
               ast::makeExpr(ast::TypeCast{std::move(expr), type_name.getStringValue()}, cast_loc);
         } else {
            break;
         }
      }

      // Handle comparison operators
      if (check(TokenType::Equals) || check(TokenType::NotEquals) || check(TokenType::LessThan) ||
          check(TokenType::LessEqual) || check(TokenType::GreaterThan) ||
          check(TokenType::GreaterEqual)) {
         SourceLocation op_loc = current().location;
         ast::BinaryOp op;
         switch (current().type) {
            case TokenType::Equals:
               op = ast::BinaryOp::Equals;
               break;
            case TokenType::NotEquals:
               op = ast::BinaryOp::NotEquals;
               break;
            case TokenType::LessThan:
               op = ast::BinaryOp::LessThan;
               break;
            case TokenType::LessEqual:
               op = ast::BinaryOp::LessEqual;
               break;
            case TokenType::GreaterThan:
               op = ast::BinaryOp::GreaterThan;
               break;
            case TokenType::GreaterEqual:
               op = ast::BinaryOp::GreaterEqual;
               break;
            default:
               break;
         }
         advance();
         auto right = parsePostfixExpr();
         expr = ast::makeExpr(ast::BinaryExpr{op, std::move(expr), std::move(right)}, op_loc);
      }

      // Handle && and || at the argument level
      while (check(TokenType::And)) {
         SourceLocation and_loc = current().location;
         advance();
         auto right = parseNotExpr();
         expr = ast::makeExpr(
            ast::BinaryExpr{ast::BinaryOp::And, std::move(expr), std::move(right)}, and_loc
         );
      }

      while (check(TokenType::Or)) {
         SourceLocation or_loc = current().location;
         advance();
         auto right = parseAndExpr();
         expr = ast::makeExpr(
            ast::BinaryExpr{ast::BinaryOp::Or, std::move(expr), std::move(right)}, or_loc
         );
      }

      return ast::Argument{std::nullopt, std::move(expr), loc};
   }

   auto value = parseExpression();
   return ast::Argument{std::nullopt, std::move(value), loc};
}

}  // namespace silo::query_engine::saneql
