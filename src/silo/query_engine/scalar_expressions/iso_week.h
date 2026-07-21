#pragma once

#include <memory>
#include <string>
#include <vector>

#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::scalar_expressions {

/// A scalar expression that evaluates to the ISO week number (int) of a date-valued
/// column, e.g. `myDate.isoWeek()`. Its value is an int, so it is not a filter predicate;
/// compile() is unimplemented and it is only meaningful as a scalar expression (e.g. in a
/// map() assignment).
class IsoWeek : public ScalarExpression {
  public:
   schema::ColumnIdentifier input_column;

   explicit IsoWeek(schema::ColumnIdentifier input_column);

   [[nodiscard]] schema::ColumnType type() const override { return schema::ColumnType::INT64; }

   static constexpr Kind KIND = Kind::ISO_WEEK;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<IsoWeek>(input_column);
   }

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
