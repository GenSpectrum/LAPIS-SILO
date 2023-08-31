#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace roaring {
class Roaring;
}  // namespace roaring

namespace silo::query_engine::operators {

class BitmapSelection : public Operator {
  public:
   enum Comparator { CONTAINS, NOT_CONTAINS };

  private:
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

   ~BitmapSelection() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators
