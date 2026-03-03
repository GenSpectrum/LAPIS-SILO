#include "silo/query_engine/saneql/lexer.h"

#include <cctype>
#include <charconv>

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
   char current = input[position];
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
      char current = peek();
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

Token Lexer::makeToken(TokenType type, SourceLocation loc) const {
   return Token{type, std::monostate{}, loc};
}

Token Lexer::makeToken(TokenType type, TokenValue value, SourceLocation loc) const {
   return Token{type, std::move(value), loc};
}

Token Lexer::readString() {
   SourceLocation start = current_location;
   advance();  // consume opening quote

   std::string result;
   while (!isAtEnd() && peek() != '\'') {
      if (peek() == '\\') {
         advance();
         if (isAtEnd()) {
            throw ParseException("Unterminated string literal", start);
         }
         char escaped = advance();
         switch (escaped) {
            case '\'':
               result += '\'';
               break;
            case '\\':
               result += '\\';
               break;
            case 'n':
               result += '\n';
               break;
            case 't':
               result += '\t';
               break;
            default:
               result += '\\';
               result += escaped;
               break;
         }
      } else {
         result += advance();
      }
   }

   if (isAtEnd()) {
      throw ParseException("Unterminated string literal", start);
   }
   advance();  // consume closing quote

   return makeToken(TokenType::StringLiteral, std::move(result), start);
}

Token Lexer::readNumber() {
   SourceLocation start = current_location;
   size_t num_start = position;

   if (peek() == '-') {
      advance();
   }

   while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
      advance();
   }

   bool is_float = false;
   if (!isAtEnd() && peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
      is_float = true;
      advance();  // consume '.'
      while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) {
         advance();
      }
   }

   std::string_view num_str = input.substr(num_start, position - num_start);

   if (is_float) {
      double val = 0;
      auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
      if (ec != std::errc()) {
         throw ParseException("Invalid float literal", start);
      }
      return makeToken(TokenType::FloatLiteral, val, start);
   }

   int64_t val = 0;
   auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
   if (ec != std::errc()) {
      throw ParseException("Invalid integer literal", start);
   }
   return makeToken(TokenType::IntLiteral, val, start);
}

Token Lexer::readIdentifierOrKeyword() {
   SourceLocation start = current_location;
   size_t id_start = position;

   while (!isAtEnd() &&
          (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_' || peek() == '.')) {
      // Only allow dot if followed by alnum or underscore (for qualified names like segment1.A123T)
      if (peek() == '.') {
         // Don't consume dot as part of identifier - let it be a separate token
         break;
      }
      advance();
   }

   std::string identifier(input.substr(id_start, position - id_start));

   if (identifier == "true") {
      return makeToken(TokenType::BoolLiteral, true, start);
   }
   if (identifier == "false") {
      return makeToken(TokenType::BoolLiteral, false, start);
   }
   if (identifier == "null") {
      return makeToken(TokenType::NullLiteral, std::monostate{}, start);
   }

   return makeToken(TokenType::Identifier, std::move(identifier), start);
}

Token Lexer::nextToken() {
   skipWhitespace();

   if (isAtEnd()) {
      return makeToken(TokenType::Eof, current_location);
   }

   SourceLocation start = current_location;
   char current = peek();

   if (current == '\'') {
      return readString();
   }

   if (std::isdigit(static_cast<unsigned char>(current))) {
      return readNumber();
   }

   if (current == '-' && position + 1 < input.size() &&
       std::isdigit(static_cast<unsigned char>(input[position + 1]))) {
      return readNumber();
   }

   if (std::isalpha(static_cast<unsigned char>(current)) || current == '_') {
      return readIdentifierOrKeyword();
   }

   switch (current) {
      case '.':
         advance();
         return makeToken(TokenType::Dot, start);
      case ',':
         advance();
         return makeToken(TokenType::Comma, start);
      case '(':
         advance();
         return makeToken(TokenType::LeftParen, start);
      case ')':
         advance();
         return makeToken(TokenType::RightParen, start);
      case '{':
         advance();
         return makeToken(TokenType::LeftBrace, start);
      case '}':
         advance();
         return makeToken(TokenType::RightBrace, start);
      case '!':
         advance();
         return makeToken(TokenType::Not, start);
      case '=':
         advance();
         return makeToken(TokenType::Equals, start);
      case '<':
         advance();
         if (!isAtEnd() && peek() == '>') {
            advance();
            return makeToken(TokenType::NotEquals, start);
         }
         if (!isAtEnd() && peek() == '=') {
            advance();
            return makeToken(TokenType::LessEqual, start);
         }
         return makeToken(TokenType::LessThan, start);
      case '>':
         advance();
         if (!isAtEnd() && peek() == '=') {
            advance();
            return makeToken(TokenType::GreaterEqual, start);
         }
         return makeToken(TokenType::GreaterThan, start);
      case '&':
         advance();
         if (!isAtEnd() && peek() == '&') {
            advance();
            return makeToken(TokenType::And, start);
         }
         throw ParseException("Expected '&&'", start);
      case '|':
         advance();
         if (!isAtEnd() && peek() == '|') {
            advance();
            return makeToken(TokenType::Or, start);
         }
         throw ParseException("Expected '||'", start);
      case ':':
         advance();
         if (!isAtEnd() && peek() == ':') {
            advance();
            return makeToken(TokenType::DoubleColon, start);
         }
         if (!isAtEnd() && peek() == '=') {
            advance();
            return makeToken(TokenType::ColonEquals, start);
         }
         throw ParseException("Expected '::' or ':='", start);
      default:
         advance();
         throw ParseException(fmt::format("Unexpected character '{}'", current), start);
   }
}

std::vector<Token> Lexer::tokenizeAll() {
   std::vector<Token> tokens;
   while (true) {
      Token token = nextToken();
      tokens.push_back(token);
      if (token.type == TokenType::Eof) {
         break;
      }
   }
   return tokens;
}

}  // namespace silo::query_engine::saneql
