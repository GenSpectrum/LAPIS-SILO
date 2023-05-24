#include "silo/query_engine/operators/range_selection.h"

#include <boost/algorithm/string/join.hpp>
#include <roaring/roaring.hh>
#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

RangeSelection::RangeSelection(std::vector<Range>&& ranges, unsigned sequence_count)
    : ranges(std::move(ranges)),
      sequence_count(sequence_count) {}

RangeSelection::~RangeSelection() noexcept = default;

std::string RangeSelection::toString() const {
   std::vector<std::string> range_strings;
   std::transform(
      ranges.begin(),
      ranges.end(),
      std::back_inserter(range_strings),
      [](const RangeSelection::Range& range) {
         return std::to_string(range.from) + "-" + std::to_string(range.to);
      }
   );
   return "RangeSelection(" + boost::algorithm::join(range_strings, ", ") + ")";
}

Type RangeSelection::type() const {
   return RANGE_SELECTION;
}

void RangeSelection::negate() {
   std::vector<Range> new_ranges;
   const unsigned last_to = 0;
   for (auto& current : ranges) {
      if (last_to != current.from) {
         new_ranges.push_back({last_to, current.from});
      }
   }
   if (last_to != sequence_count) {
      new_ranges.push_back({last_to, sequence_count});
   }
   ranges = new_ranges;
}

OperatorResult RangeSelection::evaluate() const {
   auto* result = new roaring::Roaring();
   for (const auto& range : ranges) {
      result->addRange(range.from, range.to);
   }
   return OperatorResult(result);
}

}  // namespace silo::query_engine::operators
