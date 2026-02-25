#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "silo/query_engine/saneql/source_location.h"

namespace silo::query_engine::saneql {

enum class TokenType : uint8_t {
   IntLiteral,
   FloatLiteral,
   StringLiteral,
   BoolLiteral,
   NullLiteral,
   Identifier,
   Dot,
   DoubleColon,
   ColonEquals,
   Equals,
   NotEquals,
   LessThan,
   LessEqual,
   GreaterThan,
   GreaterEqual,
   And,
   Or,
   Not,
   LeftParen,
   RightParen,
   LeftBrace,
   RightBrace,
   Comma,
   Eof,
   Error
};

[[nodiscard]] std::string tokenTypeToString(TokenType type);

using TokenValue = std::variant<std::monostate, int64_t, double, std::string, bool>;

struct Token {
   TokenType type;
   TokenValue value;
   SourceLocation location;

   [[nodiscard]] std::string toString() const;

   [[nodiscard]] std::string getStringValue() const;
   [[nodiscard]] int64_t getIntValue() const;
   [[nodiscard]] double getFloatValue() const;
   [[nodiscard]] bool getBoolValue() const;
};

}  // namespace silo::query_engine::saneql
