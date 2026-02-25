#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "silo/query_engine/saneql/source_location.h"

namespace silo::query_engine::saneql {

enum class TokenType : uint8_t {
   INT_LITERAL,
   FLOAT_LITERAL,
   STRING_LITERAL,
   BOOL_LITERAL,
   NULL_LITERAL,
   IDENTIFIER,
   DOT,
   DOUBLE_COLON,
   COLON_EQUALS,
   EQUALS,
   NOT_EQUALS,
   LESS_THAN,
   LESS_EQUAL,
   GREATER_THAN,
   GREATER_EQUAL,
   AND,
   OR,
   NOT,
   LEFT_PAREN,
   RIGHT_PAREN,
   LEFT_BRACE,
   RIGHT_BRACE,
   COMMA,
   END_OF_FILE
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
