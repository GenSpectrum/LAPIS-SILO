#pragma once

#include <string_view>
#include <vector>

#include "silo/query_engine/saneql/source_location.h"
#include "silo/query_engine/saneql/token.h"

namespace silo::query_engine::saneql {

class Lexer {
   std::string_view input;
   size_t position = 0;
   SourceLocation current_location;

  public:
   explicit Lexer(std::string_view input);

   [[nodiscard]] Token nextToken();
   [[nodiscard]] std::vector<Token> tokenizeAll();

  private:
   [[nodiscard]] char peek() const;
   [[nodiscard]] char peekNext() const;
   char advance();
   void skipWhitespace();
   [[nodiscard]] bool isAtEnd() const;

   [[nodiscard]] static Token makeToken(TokenType type, SourceLocation loc);
   [[nodiscard]] static Token makeToken(TokenType type, TokenValue value, SourceLocation loc);

   [[nodiscard]] Token readString();
   [[nodiscard]] Token readQuotedIdentifier();
   [[nodiscard]] Token readNumber();
   [[nodiscard]] Token readIdentifierOrKeyword();
};

}  // namespace silo::query_engine::saneql
