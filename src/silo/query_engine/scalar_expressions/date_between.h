#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/common/date32.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/range_selection.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"
#include "silo/storage/column/date32_column.h"

namespace silo::query_engine::scalar_expressions {

class DateBetween : public ScalarExpression {
  private:
   std::string column_name;
   std::optional<silo::common::Date32> date_from;
   std::optional<silo::common::Date32> date_to;

   [[nodiscard]] std::vector<silo::query_engine::filter::operators::RangeSelection::Range>
   computeRangesOfSortedColumn(const silo::storage::column::Date32Column& date_column) const;

  public:
   explicit DateBetween(
      std::string column_name,
      std::optional<silo::common::Date32> date_from,
      std::optional<silo::common::Date32> date_to
   );

   [[nodiscard]] std::unique_ptr<ScalarExpression> clone() const override {
      return std::make_unique<DateBetween>(column_name, date_from, date_to);
   }

   [[nodiscard]] std::string toString() const override;
   static constexpr Kind KIND = Kind::DATE_BETWEEN;
   [[nodiscard]] Kind kind() const override { return KIND; }

   [[nodiscard]] std::unique_ptr<ScalarExpression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<filter::operators::Operator> compile(const storage::Table& table
   ) const override;
};

}  // namespace silo::query_engine::scalar_expressions
