#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "silo/query_engine/saneql/source_location.h"

namespace silo::query_engine::saneql::ast {

struct Expression;

using ExpressionPtr = std::unique_ptr<Expression>;

struct Argument {
   std::optional<std::string> name;
   ExpressionPtr value;
   SourceLocation location;
};

enum class BinaryOp : uint8_t {
   And,
   Or,
   Equals,
   NotEquals,
   LessThan,
   LessEqual,
   GreaterThan,
   GreaterEqual
};

[[nodiscard]] std::string binaryOpToString(BinaryOp op);

struct IntLiteral {
   int64_t value;
};

struct FloatLiteral {
   double value;
};

struct StringLiteral {
   std::string value;
};

struct BoolLiteral {
   bool value;
};

struct NullLiteral {};

struct Identifier {
   std::string name;
};

struct BinaryExpr {
   BinaryOp op;
   ExpressionPtr left;
   ExpressionPtr right;
};

struct UnaryNotExpr {
   ExpressionPtr operand;
};

struct MethodCall {
   ExpressionPtr receiver;
   std::string method_name;
   std::vector<Argument> arguments;
};

struct FunctionCall {
   std::string function_name;
   std::vector<Argument> arguments;
};

struct TypeCast {
   ExpressionPtr operand;
   std::string target_type;
};

struct SetLiteral {
   std::vector<ExpressionPtr> elements;
};

using ExpressionVariant = std::variant<
   IntLiteral,
   FloatLiteral,
   StringLiteral,
   BoolLiteral,
   NullLiteral,
   Identifier,
   BinaryExpr,
   UnaryNotExpr,
   MethodCall,
   FunctionCall,
   TypeCast,
   SetLiteral>;

struct Expression {
   ExpressionVariant value;
   SourceLocation location;

   [[nodiscard]] std::string toString() const;
};

ExpressionPtr makeExpr(ExpressionVariant value, SourceLocation location);

}  // namespace silo::query_engine::saneql::ast
