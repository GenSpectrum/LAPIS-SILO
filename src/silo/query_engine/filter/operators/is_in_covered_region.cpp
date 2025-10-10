#include "silo/query_engine/filter/operators/is_in_covered_region.h"

#include <string>

#include <fmt/format.h>
#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/common/panic.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

IsInCoveredRegion::IsInCoveredRegion(
   const std::vector<std::pair<size_t, size_t>>* covered_region_ranges,
   const std::map<size_t, roaring::Roaring>* covered_region_bitmaps,
   uint32_t row_count,
   Comparator comparator,
   uint32_t value
)
    : covered_region_ranges(covered_region_ranges),
      covered_region_bitmaps(covered_region_bitmaps),
      row_count(row_count),
      comparator(comparator),
      value(value) {
   SILO_ASSERT_EQ(row_count, covered_region_ranges->size());
}

IsInCoveredRegion::~IsInCoveredRegion() noexcept = default;

std::string IsInCoveredRegion::toString() const {
   return fmt::format("IsInCoveredRegion({})", value);
}

Type IsInCoveredRegion::type() const {
   return IS_IN_COVERED_REGION;
}

CopyOnWriteBitmap IsInCoveredRegion::evaluate() const {
   EVOBENCH_SCOPE("IsInCoveredRegion", "evaluate");
   CopyOnWriteBitmap result_bitmap;
   auto bitmap_iter = covered_region_bitmaps->begin();
   for (size_t row_idx = 0; row_idx < row_count; ++row_idx) {
      const auto& [start, end] = covered_region_ranges->at(row_idx);
      std::optional<const roaring::Roaring*> row_bitmap = std::nullopt;
      if (bitmap_iter != covered_region_bitmaps->end() && bitmap_iter->first == row_idx) {
         row_bitmap = &bitmap_iter->second;
         bitmap_iter++;
      }
      // Check whether `value` is covered -> value in [start, end) and not in row_bitmap
      if (start <= value && value < end) {
         if (!row_bitmap.has_value() || !row_bitmap.value()->contains(value)) {
            result_bitmap->add(row_idx);
         }
      }
   }
   if (comparator == CONTAINS) {
      result_bitmap->flip(0, row_count);
   }
   return result_bitmap;
}

std::unique_ptr<Operator> IsInCoveredRegion::negate(
   std::unique_ptr<IsInCoveredRegion>&& is_in_covered_region
) {
   switch (is_in_covered_region->comparator) {
      case CONTAINS:
         is_in_covered_region->comparator = NOT_CONTAINS;
         break;
      case NOT_CONTAINS:
         is_in_covered_region->comparator = CONTAINS;
         break;
   }
   return is_in_covered_region;
}

}  // namespace silo::query_engine::filter::operators
