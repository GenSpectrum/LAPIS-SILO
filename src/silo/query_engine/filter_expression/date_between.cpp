#include "silo/query_engine/filter_expressions/date_between.h"

#include "silo/query_engine/filter_expressions/expression.h"
#include "silo/query_engine/operators/range_selection.h"

#include "silo/storage/database_partition.h"

namespace operators = silo::query_engine::operators;

namespace silo::query_engine::filter_expressions {

DateBetween::DateBetween(time_t date_from, bool open_from, time_t date_to, bool open_to)
    : date_from(date_from),
      open_from(open_from),
      date_to(date_to),
      open_to(open_to) {}

std::string DateBetween::toString(const silo::Database& /*database*/) {
   std::string res = "[Date-between ";
   res += (open_from ? "unbounded" : std::to_string(date_from));
   res += " and ";
   res += (open_to ? "unbounded" : std::to_string(date_to));
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
      const auto* lower = open_from ? begin : std::lower_bound(begin, end, this->date_from);
      uint32_t const lower_index = lower - base;
      const auto* upper = open_to ? end : std::upper_bound(begin, end, this->date_to);
      uint32_t const upper_index = upper - base;
      ranges.push_back({lower_index, upper_index});
   }
   return std::make_unique<operators::RangeSelection>(
      std::move(ranges), database_partition.sequenceCount
   );
}

}  // namespace silo::query_engine::filter_expressions
