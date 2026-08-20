#pragma once

#include <memory>
#include <string>
#include <vector>

#include "rhydb/query_engine/filter/operators/operator.h"
#include "rhydb/query_engine/scalar_expressions/scalar_expression.h"
#include "rhydb/schema/database_schema.h"

namespace rhydb::query_engine::scalar_expressions {

/// A scalar expression that evaluates to the ISO 8601 week date of the date its child expression
/// evaluates to, formatted as `<ISO-year>-W<ISO-week>`, e.g. `2026-W12`. The ISO week-numbering
/// year
/// (`%G`) is used, so it can differ from the calendar year around January (e.g. `2021-01-01` is
/// `2020-W53`). The child is usually a `FieldRef` to a date column. Its value is a string, so it is
/// not a filter predicate; compile() is unimplemented and it is only meaningful as a scalar
/// expression (e.g. in a map() assignment).
class IsoWeek : public ScalarExpression {
  public:
   std::unique_ptr<ScalarExpression> input;

   explicit IsoWeek(std::unique_ptr<ScalarExpression> input);

   [[nodiscard]] schema::ColumnType type() const override { return schema::ColumnType::STRING; }

   static constexpr Kind KIND = Kind::ISO_WEEK;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::vector<schema::ColumnIdentifier> freeIUs() const override;

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<IsoWeek>(input->clone());
   }

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace rhydb::query_engine::scalar_expressions
