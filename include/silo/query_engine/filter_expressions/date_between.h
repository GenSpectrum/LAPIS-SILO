#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/date.h"
#include "silo/database.h"
#include "silo/preprocessing/partition.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/range_selection.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

class DateBetween : public Expression {
  private:
   std::string column_name;
   std::optional<silo::common::Date> date_from;
   std::optional<silo::common::Date> date_to;

   [[nodiscard]] std::vector<silo::query_engine::operators::RangeSelection::Range>
   computeRangesOfSortedColumn(
      const silo::storage::column::DateColumnPartition& date_column,
      const std::vector<silo::preprocessing::PartitionChunk>& chunks
   ) const;

  public:
   explicit DateBetween(
      std::string column_name,
      std::optional<silo::common::Date> date_from,
      std::optional<silo::common::Date> date_to
   );

   std::string toString() const override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition,
      AmbiguityMode mode
   ) const override;
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<DateBetween>& filter);

}  // namespace silo::query_engine::filter_expressions
