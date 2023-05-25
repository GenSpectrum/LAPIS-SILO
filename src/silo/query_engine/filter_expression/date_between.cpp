#include "silo/query_engine/filter_expressions/date_between.h"

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/range_selection.h"

#include "silo/storage/database_partition.h"

namespace silo::query_engine::filter_expressions {

DateBetween::DateBetween(std::optional<time_t> date_from, std::optional<time_t> date_to)
    : date_from(date_from),
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
   std::vector<operators::RangeSelection::Range> ranges;

   const auto* base = database_partition.meta_store.sequence_id_to_date.data();
   for (const auto& chunk : database_partition.getChunks()) {
      const auto* begin = &database_partition.meta_store.sequence_id_to_date[chunk.offset];
      const auto* end = &database_partition.meta_store
                            .sequence_id_to_date[chunk.offset + chunk.count_of_sequences];
      const auto* lower =
         date_from.has_value() ? std::lower_bound(begin, end, date_from.value()) : begin;
      uint32_t const lower_index = lower - base;
      const auto* upper = date_to.has_value() ? std::upper_bound(begin, end, date_to.value()) : end;
      uint32_t const upper_index = upper - base;
      ranges.emplace_back(lower_index, upper_index);
   }
   return std::make_unique<operators::RangeSelection>(
      std::move(ranges), database_partition.sequenceCount
   );
}

}  // namespace silo::query_engine::filter_expressions
