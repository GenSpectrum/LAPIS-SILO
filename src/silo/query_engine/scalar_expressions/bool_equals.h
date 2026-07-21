#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

struct BoolEquals : public ScalarExpression {
  private:
   std::string column_name;
   std::optional<bool> value;

  public:
   explicit BoolEquals(std::string column_name, std::optional<bool> value);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<BoolEquals>(column_name, value);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::BOOL_EQUALS;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
