#include "silo/query_engine/saneql/lexer.h"

#include <cctype>

#include <fast_float/fast_float.h>
#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/query_engine/saneql/parse_exception.h"

namespace silo::query_engine::saneql {

Lexer::Lexer(std::string_view input)
    : input(input) {}

bool Lexer::isAtEnd() const {
   return position >= input.size();
}

char Lexer::peek() const {
   if (isAtEnd()) {
      return '\0';
   }
   return input[position];
}

char Lexer::peekNext() const {
   if (position + 1 >= input.size()) {
      return '\0';
   }
   return input[position + 1];
}

char Lexer::advance() {
   SILO_ASSERT(!isAtEnd());
   const char current = input[position];
   position++;
   if (current == '\n') {
      current_location.line++;
      current_location.column = 1;
   } else {
      current_location.column++;
   }
   return current;
}

void Lexer::skipWhitespace() {
   while (!isAtEnd()) {
      const char current = peek();
      if (current == ' ' || current == '\t' || current == '\n' || current == '\r') {
         advance();
      } else if (current == '-' && peekNext() == '-') {
         while (!isAtEnd() && peek() != '\n') {
            advance();
         }
      } else {
         break;
      }
   }
}

Token Lexer::makeToken(TokenType type, SourceLocation loc) {
   return makeToken(type, std::monostate{}, loc);
}

Token Lexer::makeToken(TokenType type, TokenValue value, SourceLocation loc) {
   return Token{.type = type, .value = std::move(value), .location = loc};
}

Token Lexer::readString() {
   const SourceLocation start = current_location;
   SILO_ASSERT(peek() == '\'');
   advance();

   std::string result;
   while (!isAtEnd()) {
      if (peek() == '\'') {
         advance();
         if (!isAtEnd() && peek() == '\'') {
            advance();
            // Escaped single quote: '' -> '
            result += '\'';
         } else {
            return makeToken(TokenType::STRING_LITERAL, std::move(result), start);
         }
      } else {
         result += advance();
      }
   }
   throw ParseException(start, "Unterminated string literal");
}

Token Lexer::readQuotedIdentifier() {
   const SourceLocation start = current_location;

   SILO_ASSERT(peek() == '"');
   advance();

   std::string result;
   while (!isAtEnd()) {
      if (peek() == '"') {
         advance();
         if (!isAtEnd() && peek() == '"') {
            advance();
            // Escaped double quote: "" -> "
            result += '"';
         } else {
            return makeToken(TokenType::IDENTIFIER, std::move(result), start);
         }
      } else {
         result += advance();
      }
   }
   throw ParseException(start, "Unterminated quoted identifier");
}

Token Lexer::readNumber() {
   const SourceLocation start = current_location;
   const size_t num_start = position;

   // TODO(#1246) do not lex negative numbers
   if (peek() == '-') {
      advance();
   }

   bool is_float = false;
   while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '.')) {
      if (peek() == '.') {
         is_float = true;
      }
      advance();
   }

   const std::string_view num_str = input.substr(num_start, position - num_start);

   if (is_float) {
      double val = 0;
      auto [ptr, ec] = fast_float::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
      if (ec != std::errc() || ptr != num_str.end()) {
         throw ParseException(start, "Invalid float literal");
      }
      return makeToken(TokenType::FLOAT_LITERAL, val, start);
   }

   int64_t val = 0;
   auto [ptr, ec] = fast_float::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
   if (ec != std::errc() || ptr != num_str.end()) {
      throw ParseException(start, "Invalid integer literal");
   }
   return makeToken(TokenType::INT_LITERAL, val, start);
}

Token Lexer::readIdentifierOrKeyword() {
   const SourceLocation start = current_location;
   const size_t id_start = position;

   while (!isAtEnd() && (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')) {
      advance();
   }

   std::string identifier(input.substr(id_start, position - id_start));

   if (identifier == "true") {
      return makeToken(TokenType::BOOL_LITERAL, true, start);
   }
   if (identifier == "false") {
      return makeToken(TokenType::BOOL_LITERAL, false, start);
   }
   if (identifier == "null") {
      return makeToken(TokenType::NULL_LITERAL, std::monostate{}, start);
   }

   return makeToken(TokenType::IDENTIFIER, std::move(identifier), start);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
Token Lexer::nextToken() {
   skipWhitespace();

   if (isAtEnd()) {
      return makeToken(TokenType::END_OF_FILE, current_location);
   }

   const SourceLocation start = current_location;
   char current = peek();

   if (current == '"') {
      return readQuotedIdentifier();
   }

   if (current == '\'') {
      return readString();
   }

   if (std::isdigit(static_cast<unsigned char>(current))) {
      return readNumber();
   }

   if (current == '-' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
      return readNumber();
   }

   if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
      return readIdentifierOrKeyword();
   }

   switch (current) {
      case '.':
         advance();
         return makeToken(TokenType::DOT, start);
      case ',':
         advance();
         return makeToken(TokenType::COMMA, start);
      case '(':
         advance();
         return makeToken(TokenType::LEFT_PAREN, start);
      case ')':
         advance();
         return makeToken(TokenType::RIGHT_PAREN, start);
      case '{':
         advance();
         return makeToken(TokenType::LEFT_BRACE, start);
      case '}':
         advance();
         return makeToken(TokenType::RIGHT_BRACE, start);
      case '!':
         advance();
         return makeToken(TokenType::NOT, start);
      case '=':
         advance();
         return makeToken(TokenType::EQUALS, start);
      case '<':
         advance();
         if (!isAtEnd() && peek() == '>') {
            advance();
            return makeToken(TokenType::NOT_EQUALS, start);
         }
         if (!isAtEnd() && peek() == '=') {
            advance();
            return makeToken(TokenType::LESS_EQUAL, start);
         }
         return makeToken(TokenType::LESS_THAN, start);
      case '>':
         advance();
         if (!isAtEnd() && peek() == '=') {
            advance();
            return makeToken(TokenType::GREATER_EQUAL, start);
         }
         return makeToken(TokenType::GREATER_THAN, start);
      case '&':
         advance();
         if (!isAtEnd() && peek() == '&') {
            advance();
            return makeToken(TokenType::AND, start);
         }
         throw ParseException(start, "Expected '&&'");
      case '|':
         advance();
         if (!isAtEnd() && peek() == '|') {
            advance();
            return makeToken(TokenType::OR, start);
         }
         throw ParseException(start, "Expected '||'");
      case ':':
         advance();
         if (!isAtEnd() && peek() == ':') {
            advance();
            return makeToken(TokenType::DOUBLE_COLON, start);
         }
         if (!isAtEnd() && peek() == '=') {
            advance();
            return makeToken(TokenType::COLON_EQUALS, start);
         }
         throw ParseException(start, "Expected '::' or ':='");
      default:
         advance();
         throw ParseException(start, "Unexpected character '{}'", current);
   }
}

std::vector<Token> Lexer::tokenizeAll() {
   std::vector<Token> tokens;
   while (true) {
      const Token token = nextToken();
      tokens.push_back(token);
      if (token.type == TokenType::END_OF_FILE) {
         break;
      }
   }
   return tokens;
}

}  // namespace silo::query_engine::saneql
