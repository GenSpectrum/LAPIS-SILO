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
   enum Comparator { CONTAINS, NOT_CONTAINS };

  private:
   const std::vector<std::pair<size_t, size_t>>* covered_region_ranges;
   const std::map<size_t, roaring::Roaring>* covered_region_bitmaps;
   uint32_t row_count;
   Comparator comparator;
   uint32_t value;

  public:
   explicit IsInCoveredRegion(
      const std::vector<std::pair<size_t, size_t>>* covered_region_ranges,
      const std::map<size_t, roaring::Roaring>* covered_region_bitmaps,
      uint32_t row_count,
      Comparator comparator,
      uint32_t value
   );

   ~IsInCoveredRegion() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual CopyOnWriteBitmap evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<IsInCoveredRegion>&& bitmap_selection);
};

}  // namespace silo::query_engine::filter::operators
