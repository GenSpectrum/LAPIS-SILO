#ifndef SILO_SEQUENCE_SELECTION_H
#define SILO_SEQUENCE_SELECTION_H

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class BitmapSelection : public Operator {
  public:
   enum Predicate { CONTAINS, NOT_CONTAINS };

  private:
   const roaring::Roaring* bitmaps;
   unsigned sequence_count;
   Predicate comparator;
   uint32_t value;

  public:
   explicit BitmapSelection(
      const roaring::Roaring* bitmaps,
      unsigned sequence_count,
      Predicate predicate,
      uint32_t value
   );

   ~BitmapSelection() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual void negate();
};

}  // namespace silo::query_engine::operators

#endif  // SILO_SEQUENCE_SELECTION_H
