#include "silo/query_engine/filter_expressions/date_between.h"

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/range_selection.h"

#include "silo/query_engine/operators/intersection.h"
#include "silo/query_engine/operators/operator.h"
#include "silo/query_engine/operators/selection.h"
#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

DateBetween::DateBetween(
   std::string column,
   std::optional<time_t> date_from,
   std::optional<time_t> date_to
)
    : column(std::move(column)),
      date_from(date_from),
      date_to(date_to) {}

std::string DateBetween::toString(const silo::Database& /*database*/) {
   std::string res = "[Date-between ";
   res += (date_from.has_value() ? std::to_string(date_from.value()) : "unbounded");
   res += " and ";
   res += (date_to.has_value() ? std::to_string(date_to.value()) : "unbounded");
   res += "]";
   return res;
}

std::unique_ptr<operators::Operator> DateBetween::compile(
   const silo::Database& /*database*/,
   const silo::DatabasePartition& database_partition
) const {
   const auto& date_column = database_partition.meta_store.date_columns.at(column);

   if (!date_column.isSorted()) {
      std::vector<std::unique_ptr<operators::Operator>> children;
      children.emplace_back(std::make_unique<operators::Selection<time_t>>(
         date_column.getValues(),
         operators::Selection<time_t>::HIGHER_OR_EQUALS,
         date_from.value_or(0),
         database_partition.sequenceCount
      ));
      children.emplace_back(std::make_unique<operators::Selection<time_t>>(
         date_column.getValues(),
         operators::Selection<time_t>::LESS,
         date_to.value_or(std::numeric_limits<time_t>::max()),
         database_partition.sequenceCount
      ));

      return std::make_unique<operators::Intersection>(
         std::move(children),
         std::vector<std::unique_ptr<operators::Operator>>(),
         database_partition.sequenceCount
      );
   }

   return std::make_unique<operators::RangeSelection>(
      computeRangesOfSortedColumn(date_column, database_partition.getChunks()),
      database_partition.sequenceCount
   );
}

std::vector<silo::query_engine::operators::RangeSelection::Range> DateBetween::
   computeRangesOfSortedColumn(
      const silo::storage::column::DateColumn& date_column,
      const std::vector<silo::preprocessing::Chunk>& chunks
   ) const {
   std::vector<operators::RangeSelection::Range> ranges;

   const auto* base = date_column.getValues().data();
   for (const auto& chunk : chunks) {
      const auto* begin = &date_column.getValues()[chunk.offset];
      const auto* end = &date_column.getValues()[chunk.offset + chunk.count_of_sequences];
      const auto* lower =
         date_from.has_value() ? std::lower_bound(begin, end, date_from.value()) : begin;
      uint32_t const lower_index = lower - base;
      const auto* upper = date_to.has_value() ? std::upper_bound(begin, end, date_to.value()) : end;
      uint32_t const upper_index = upper - base;
      ranges.emplace_back(lower_index, upper_index);
   }
   return ranges;
}

}  // namespace silo::query_engine::filter_expressions
