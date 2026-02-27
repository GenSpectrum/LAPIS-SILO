#include "silo/query_engine/saneql/ast.h"

#include <fmt/format.h>

namespace silo::query_engine::saneql::ast {

std::string binaryOpToString(BinaryOp op) {
   switch (op) {
      case BinaryOp::And:
         return "&&";
      case BinaryOp::Or:
         return "||";
      case BinaryOp::Equals:
         return "=";
      case BinaryOp::NotEquals:
         return "<>";
      case BinaryOp::LessThan:
         return "<";
      case BinaryOp::LessEqual:
         return "<=";
      case BinaryOp::GreaterThan:
         return ">";
      case BinaryOp::GreaterEqual:
         return ">=";
   }
   return "?";
}

namespace {

struct ExprToString {
   std::string operator()(const IntLiteral& lit) const { return std::to_string(lit.value); }

   std::string operator()(const FloatLiteral& lit) const { return fmt::format("{}", lit.value); }

   std::string operator()(const StringLiteral& lit) const { return fmt::format("'{}'", lit.value); }

   std::string operator()(const BoolLiteral& lit) const { return lit.value ? "true" : "false"; }

   std::string operator()(const NullLiteral& /*unused*/) const { return "null"; }

   std::string operator()(const Identifier& id) const { return id.name; }

   std::string operator()(const BinaryExpr& expr) const {
      return fmt::format(
         "({} {} {})", expr.left->toString(), binaryOpToString(expr.op), expr.right->toString()
      );
   }

   std::string operator()(const UnaryNotExpr& expr) const {
      return fmt::format("(!{})", expr.operand->toString());
   }

   std::string operator()(const MethodCall& call) const {
      std::string args;
      for (size_t i = 0; i < call.arguments.size(); i++) {
         if (i > 0) {
            args += ", ";
         }
         if (call.arguments[i].name.has_value()) {
            args += call.arguments[i].name.value() + ":=";
         }
         args += call.arguments[i].value->toString();
      }
      return fmt::format("{}.{}({})", call.receiver->toString(), call.method_name, args);
   }

   std::string operator()(const FunctionCall& call) const {
      std::string args;
      for (size_t i = 0; i < call.arguments.size(); i++) {
         if (i > 0) {
            args += ", ";
         }
         if (call.arguments[i].name.has_value()) {
            args += call.arguments[i].name.value() + ":=";
         }
         args += call.arguments[i].value->toString();
      }
      return fmt::format("{}({})", call.function_name, args);
   }

   std::string operator()(const TypeCast& cast) const {
      return fmt::format("{}::{}", cast.operand->toString(), cast.target_type);
   }

   std::string operator()(const SetLiteral& set) const {
      std::string elements;
      for (size_t i = 0; i < set.elements.size(); i++) {
         if (i > 0) {
            elements += ", ";
         }
         elements += set.elements[i]->toString();
      }
      return fmt::format("{{{}}}", elements);
   }
};

}  // namespace

std::string Expression::toString() const {
   return std::visit(ExprToString{}, value);
}

ExpressionPtr makeExpr(ExpressionVariant value, SourceLocation location) {
   auto expr = std::make_unique<Expression>();
   expr->value = std::move(value);
   expr->location = location;
   return expr;
}

}  // namespace silo::query_engine::saneql::ast
