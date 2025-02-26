#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "external/roaring_include_wrapper.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::expressions {
// Forward declaration for friend class access. Include would introduce cyclic dependency
class Expression;
}  // namespace silo::query_engine::filter::expressions

namespace silo::query_engine::filter::operators {

class BitmapSelection : public Operator {
   friend class Operator;

  public:
   enum Comparator { CONTAINS, NOT_CONTAINS };

  private:
   std::optional<std::unique_ptr<silo::query_engine::filter::expressions::Expression>>
      logical_equivalent;
   const roaring::Roaring* bitmaps;
   uint32_t row_count;
   Comparator comparator;
   uint32_t value;

  public:
   explicit BitmapSelection(
      const roaring::Roaring* bitmaps,
      uint32_t row_count,
      Comparator comparator,
      uint32_t value
   );

   explicit BitmapSelection(
      std::unique_ptr<silo::query_engine::filter::expressions::Expression>&& logical_equivalent,
      const roaring::Roaring* bitmaps,
      uint32_t row_count,
      Comparator comparator,
      uint32_t value
   );

   ~BitmapSelection() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual CopyOnWriteBitmap evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<BitmapSelection>&& bitmap_selection);
};

}  // namespace silo::query_engine::filter::operators
