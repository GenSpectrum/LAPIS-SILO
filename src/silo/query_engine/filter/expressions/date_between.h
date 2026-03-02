#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/common/date.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/filter/operators/range_selection.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::filter::expressions {

class DateBetween : public Expression {
  private:
   std::string column_name;
   std::optional<silo::common::Date> date_from;
   std::optional<silo::common::Date> date_to;

   [[nodiscard]] std::vector<silo::query_engine::filter::operators::RangeSelection::Range>
   computeRangesOfSortedColumn(
      const silo::storage::column::DateColumnPartition& date_column,
      const std::vector<size_t>& chunk_sizes
   ) const;

  public:
   explicit DateBetween(
      std::string column_name,
      std::optional<silo::common::Date> date_from,
      std::optional<silo::common::Date> date_to
   );

   [[nodiscard]] std::string toString() const override;

   [[nodiscard]] std::unique_ptr<Expression> rewrite(
      const storage::Table& table,
      const storage::TablePartition& table_partition,
      AmbiguityMode mode
   ) const override;

   [[nodiscard]] std::unique_ptr<operators::Operator> compile(
      const storage::Table& table,
      const storage::TablePartition& table_partition
   ) const override;
};

}  // namespace silo::query_engine::filter::expressions
