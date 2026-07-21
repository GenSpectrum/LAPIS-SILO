#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

class FloatEquals : public ScalarExpression {
  private:
   std::string column_name;
   std::optional<double> value;

  public:
   FloatEquals(std::string column_name, std::optional<double> value);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<FloatEquals>(column_name, value);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::FLOAT_EQUALS;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
