#ifndef SILO_DATE_BETWEEN_H
#define SILO_DATE_BETWEEN_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/range_selection.h"

namespace silo::storage::column {
class DateColumn;
}

namespace silo::preprocessing {
struct Chunk;
}

namespace silo::query_engine::filter_expressions {

struct DateBetween : public Expression {
  private:
   std::string column;
   std::optional<time_t> date_from;
   std::optional<time_t> date_to;

   std::vector<silo::query_engine::operators::RangeSelection::Range> computeRangesOfSortedColumn(
      const silo::storage::column::DateColumn& date_column,
      const std::vector<silo::preprocessing::Chunk>& chunks
   ) const;

  public:
   explicit DateBetween(
      std::string column,
      std::optional<time_t> date_from,
      std::optional<time_t> date_to
   );

   std::string toString(const Database& database) override;

   [[nodiscard]] std::unique_ptr<silo::query_engine::operators::Operator> compile(
      const Database& database,
      const DatabasePartition& database_partition
   ) const override;
};

}  // namespace silo::query_engine::filter_expressions

#endif  // SILO_DATE_BETWEEN_H
