#include "silo/query_engine/operators/range_selection.h"

#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string/join.hpp>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
RangeSelection::Range::Range(uint32_t start, uint32_t end)
    : start(start),
      end(end) {}

RangeSelection::RangeSelection(std::vector<Range>&& ranges, uint32_t row_count)
    : ranges(std::move(ranges)),
      row_count(row_count) {}

RangeSelection::~RangeSelection() noexcept = default;

std::string RangeSelection::toString() const {
   std::vector<std::string> range_strings;
   std::ranges::transform(
      ranges,
      std::back_inserter(range_strings),
      [](const RangeSelection::Range& range) {
         return std::to_string(range.start) + "-" + std::to_string(range.end);
      }
   );
   return "RangeSelection(" + boost::algorithm::join(range_strings, ", ") + ")";
}

Type RangeSelection::type() const {
   return RANGE_SELECTION;
}

CopyOnWriteBitmap RangeSelection::evaluate() const {
   CopyOnWriteBitmap result;
   for (const auto& range : ranges) {
      result->addRange(range.start, range.end);
   }
   return result;
}

std::unique_ptr<Operator> RangeSelection::negate(std::unique_ptr<RangeSelection>&& range_selection
) {
   const uint32_t row_count = range_selection->row_count;
   std::vector<Range> new_ranges;
   uint32_t last_to = 0;
   for (const auto& current : range_selection->ranges) {
      if (last_to != current.start) {
         new_ranges.emplace_back(last_to, current.start);
      }
      last_to = current.end;
   }
   if (last_to != row_count) {
      new_ranges.emplace_back(last_to, row_count);
   }
   return std::make_unique<RangeSelection>(std::move(new_ranges), row_count);
}

}  // namespace silo::query_engine::operators
