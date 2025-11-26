#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <roaring/roaring.hh>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {
// Forward declaration for friend class access. Include would introduce cyclic dependency
class Expression;
}  // namespace silo::query_engine::filter::expressions

namespace silo::query_engine::filter::operators {

class IsInCoveredRegion : public Operator {
   friend class Operator;

  public:
   enum class Comparator : uint8_t { COVERED, NOT_COVERED };

  private:
   const std::vector<std::pair<uint32_t, uint32_t>>* covered_region_ranges;
   const std::map<uint32_t, roaring::Roaring>* covered_region_bitmaps;
   uint32_t row_count;
   Comparator comparator;
   uint32_t value;

  public:
   explicit IsInCoveredRegion(
      const std::vector<std::pair<uint32_t, uint32_t>>* covered_region_ranges,
      const std::map<uint32_t, roaring::Roaring>* covered_region_bitmaps,
      uint32_t row_count,
      Comparator comparator,
      uint32_t value
   );

   ~IsInCoveredRegion() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<IsInCoveredRegion>&& is_in_covered_region
   );
};

}  // namespace silo::query_engine::filter::operators
