#pragma once

#include <memory>
#include <optional>
#include <string>

#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {

class FloatBetween : public Expression {
  private:
   std::string column_name;
   std::optional<double> from;
   std::optional<double> to;

  public:
   explicit FloatBetween(
      std::string column_name,
      std::optional<double> from,
      std::optional<double> to
   );

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::filter::expressions
