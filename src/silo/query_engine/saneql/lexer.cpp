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
   return Token{.type = type, .value = std::monostate{}, .location = loc};
}

Token Lexer::makeToken(TokenType type, TokenValue value, SourceLocation loc) {
   return Token{.type = type, .value = std::move(value), .location = loc};
}

Token Lexer::readString() {
   const SourceLocation start = current_location;
   advance();  // consume opening quote

   std::string result;
   while (!isAtEnd() && peek() != '\'') {
      if (peek() == '\\') {
         advance();
         if (isAtEnd()) {
            throw ParseException("Unterminated string literal", start);
         }
         const char escaped = advance();
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

   return makeToken(TokenType::STRING_LITERAL, std::move(result), start);
}

Token Lexer::readQuotedIdentifier() {
   const SourceLocation start = current_location;
   advance();  // consume opening double quote

   std::string result;
   while (!isAtEnd()) {
      if (peek() == '"') {
         advance();  // consume the quote
         if (!isAtEnd() && peek() == '"') {
            // Escaped double quote: "" → "
            result += advance();
         } else {
            // End of quoted identifier
            return makeToken(TokenType::IDENTIFIER, std::move(result), start);
         }
      } else {
         result += advance();
      }
   }

   throw ParseException("Unterminated quoted identifier", start);
}

Token Lexer::readNumber() {
   const SourceLocation start = current_location;
   const size_t num_start = position;

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

   const std::string_view num_str = input.substr(num_start, position - num_start);

   if (is_float) {
      double val = 0;
      auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
      if (ec != std::errc()) {
         throw ParseException("Invalid float literal", start);
      }
      return makeToken(TokenType::FLOAT_LITERAL, val, start);
   }

   int64_t val = 0;
   auto [ptr, ec] = std::from_chars(num_str.data(), num_str.data() + num_str.size(), val);
   if (ec != std::errc()) {
      throw ParseException("Invalid integer literal", start);
   }
   return makeToken(TokenType::INT_LITERAL, val, start);
}

Token Lexer::readIdentifierOrKeyword() {
   const SourceLocation start = current_location;
   const size_t id_start = position;

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
         throw ParseException("Expected '&&'", start);
      case '|':
         advance();
         if (!isAtEnd() && peek() == '|') {
            advance();
            return makeToken(TokenType::OR, start);
         }
         throw ParseException("Expected '||'", start);
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
         throw ParseException("Expected '::' or ':='", start);
      default:
         advance();
         throw ParseException(fmt::format("Unexpected character '{}'", current), start);
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
