#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/common/date32.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::scalar_expressions {

class DateEquals : public ScalarExpression {
  private:
   schema::ColumnIdentifier column;
   std::optional<silo::common::Date32> value;

  public:
   explicit DateEquals(schema::ColumnIdentifier column, std::optional<silo::common::Date32> value);

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<DateEquals>(column, value);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::DATE_EQUALS;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
