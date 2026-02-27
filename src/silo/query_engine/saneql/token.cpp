#include "silo/query_engine/saneql/token.h"

#include <stdexcept>

#include <fmt/format.h>

namespace silo::query_engine::saneql {

std::string tokenTypeToString(TokenType type) {
   switch (type) {
      case TokenType::IntLiteral:
         return "IntLiteral";
      case TokenType::FloatLiteral:
         return "FloatLiteral";
      case TokenType::StringLiteral:
         return "StringLiteral";
      case TokenType::BoolLiteral:
         return "BoolLiteral";
      case TokenType::NullLiteral:
         return "NullLiteral";
      case TokenType::Identifier:
         return "Identifier";
      case TokenType::Dot:
         return "Dot";
      case TokenType::DoubleColon:
         return "DoubleColon";
      case TokenType::ColonEquals:
         return "ColonEquals";
      case TokenType::Equals:
         return "Equals";
      case TokenType::NotEquals:
         return "NotEquals";
      case TokenType::LessThan:
         return "LessThan";
      case TokenType::LessEqual:
         return "LessEqual";
      case TokenType::GreaterThan:
         return "GreaterThan";
      case TokenType::GreaterEqual:
         return "GreaterEqual";
      case TokenType::And:
         return "And";
      case TokenType::Or:
         return "Or";
      case TokenType::Not:
         return "Not";
      case TokenType::LeftParen:
         return "LeftParen";
      case TokenType::RightParen:
         return "RightParen";
      case TokenType::LeftBrace:
         return "LeftBrace";
      case TokenType::RightBrace:
         return "RightBrace";
      case TokenType::Comma:
         return "Comma";
      case TokenType::Eof:
         return "Eof";
      case TokenType::Error:
         return "Error";
   }
   return "Unknown";
}

std::string Token::toString() const {
   if (type == TokenType::StringLiteral) {
      return fmt::format("Token({}, '{}')", tokenTypeToString(type), getStringValue());
   }
   if (type == TokenType::IntLiteral) {
      return fmt::format("Token({}, {})", tokenTypeToString(type), getIntValue());
   }
   if (type == TokenType::FloatLiteral) {
      return fmt::format("Token({}, {})", tokenTypeToString(type), getFloatValue());
   }
   if (type == TokenType::BoolLiteral) {
      return fmt::format(
         "Token({}, {})", tokenTypeToString(type), getBoolValue() ? "true" : "false"
      );
   }
   if (type == TokenType::Identifier) {
      return fmt::format("Token({}, {})", tokenTypeToString(type), getStringValue());
   }
   return fmt::format("Token({})", tokenTypeToString(type));
}

std::string Token::getStringValue() const {
   return std::get<std::string>(value);
}

int64_t Token::getIntValue() const {
   return std::get<int64_t>(value);
}

double Token::getFloatValue() const {
   return std::get<double>(value);
}

bool Token::getBoolValue() const {
   return std::get<bool>(value);
}

}  // namespace silo::query_engine::saneql
