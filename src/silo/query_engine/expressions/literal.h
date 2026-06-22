#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::expressions {

/// A scalar literal value, e.g. `x := 3`. Literals are scalar expressions whose
/// value is known up front; their type() reflects the literal's column type.
/// They are not filter predicates, so compile() is only meaningful for the
/// boolean literal, which compiles to a Full (true) or Empty (false) operator.

class Int64Literal : public Expression {
  public:
   int64_t value;

   explicit Int64Literal(int64_t value);

   [[nodiscard]] schema::ColumnType type() const override { return schema::ColumnType::INT64; }

   static constexpr Kind KIND = Kind::INT64_LITERAL;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<Int64Literal>(value);
   }

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

class FloatLiteral : public Expression {
  public:
   double value;

   explicit FloatLiteral(double value);

   [[nodiscard]] schema::ColumnType type() const override { return schema::ColumnType::FLOAT; }

   static constexpr Kind KIND = Kind::FLOAT_LITERAL;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<FloatLiteral>(value);
   }

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

class StringLiteral : public Expression {
  public:
   std::string value;

   explicit StringLiteral(std::string value);

   [[nodiscard]] schema::ColumnType type() const override { return schema::ColumnType::STRING; }

   static constexpr Kind KIND = Kind::STRING_LITERAL;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<StringLiteral>(value);
   }

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

class BoolLiteral : public Expression {
  public:
   bool value;

   explicit BoolLiteral(bool value);

   [[nodiscard]] schema::ColumnType type() const override { return schema::ColumnType::BOOL; }

   static constexpr Kind KIND = Kind::BOOL_LITERAL;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<BoolLiteral>(value);
   }

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
