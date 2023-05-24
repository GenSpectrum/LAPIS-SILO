#ifndef SILO_SELECTION_H
#define SILO_SELECTION_H

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class Selection : public Operator {
  public:
   enum Comparator { EQUALS, LESS, HIGHER, LESS_OR_EQUALS, HIGHER_OR_EQUALS, NOT_EQUALS };

  private:
   uint64_t const* column;
   Comparator comparator;
   uint64_t value;
   unsigned sequence_count;

  public:
   explicit Selection(
      uint64_t const* column,
      Comparator comparator,
      uint64_t value,
      unsigned sequence_count
   );

   ~Selection() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual void negate();
};

}  // namespace silo::query_engine::operators

#endif  // SILO_SELECTION_H
