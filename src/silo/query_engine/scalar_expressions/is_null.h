#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::scalar_expressions {

class IsNull : public ScalarExpression {
  private:
   std::string column_name;

  public:
   explicit IsNull(std::string column_name);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<IsNull>(column_name);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::IS_NULL;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
