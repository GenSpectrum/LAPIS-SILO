#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/common/date.h"
#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/range_selection.h"

namespace silo {
class DatabasePartition;

namespace query_engine {
namespace operators {
class Operator;
}  // namespace operators
}  // namespace query_engine
class Database;
}  // namespace silo

namespace silo::storage::column {
class DateColumnPartition;
}

namespace silo::preprocessing {
struct PartitionChunk;
}

namespace silo::query_engine::filter_expressions {

class DateBetween : public Expression {
  private:
   std::string column;
   std::optional<silo::common::Date> date_from;
   std::optional<silo::common::Date> date_to;

   [[nodiscard]] std::vector<silo::query_engine::operators::RangeSelection::Range>
   computeRangesOfSortedColumn(
      const silo::storage::column::DateColumnPartition& date_column,
      const std::vector<silo::preprocessing::PartitionChunk>& chunks
   ) const;

  public:
   explicit DateBetween(
      std::string column,
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
