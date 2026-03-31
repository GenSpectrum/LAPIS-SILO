#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/date32.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/range_selection.h"
#include "silo/storage/column/date32_column.h"

namespace silo::query_engine::filter::expressions {

class DateBetween : public Expression {
  private:
   std::string column_name;
   std::optional<silo::common::Date32> date_from;
   std::optional<silo::common::Date32> date_to;

   [[nodiscard]] std::vector<silo::query_engine::filter::operators::RangeSelection::Range>
   computeRangesOfSortedColumn(
      const silo::storage::column::Date32ColumnPartition& date_column,
      const std::vector<size_t>& chunk_sizes
   ) const;

  public:
   explicit DateBetween(
      std::string column_name,
      std::optional<silo::common::Date32> date_from,
      std::optional<silo::common::Date32> date_to
   );

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(const storage::Table& table
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<DateBetween>& filter);

}  // namespace silo::query_engine::filter::expressions
