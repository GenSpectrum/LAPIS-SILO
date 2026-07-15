#pragma once

#include <memory>
#include <string>

#include "silo/query_engine/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::expressions {

class FloatEquals : public Expression {
  private:
   std::string column_name;
   std::optional<double> value;

  public:
   FloatEquals(std::string column_name, std::optional<double> value);

   [[nodiscard]] std::unique_ptr<Expression> clone() const override {
      return std::make_unique<FloatEquals>(column_name, value);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::FLOAT_EQUALS;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::expressions
