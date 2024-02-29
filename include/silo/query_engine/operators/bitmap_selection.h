#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace roaring {
class Roaring;
}  // namespace roaring

namespace silo::query_engine::filter_expressions {
class Expression;
}  // namespace silo::query_engine::filter_expressions

namespace silo::query_engine::operators {

class BitmapSelection : public Operator {
   friend class Operator;

  public:
   enum Comparator { CONTAINS, NOT_CONTAINS };

  private:
   std::optional<std::unique_ptr<silo::query_engine::filter_expressions::Expression>>
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
      std::unique_ptr<silo::query_engine::filter_expressions::Expression>&& logical_equivalent,
      const roaring::Roaring* bitmaps,
      uint32_t row_count,
      Comparator comparator,
      uint32_t value
   );

   ~BitmapSelection() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<BitmapSelection>&& bitmap_selection);
};

}  // namespace silo::query_engine::operators
