#ifndef SILO_RANGE_SELECTION_H
#define SILO_RANGE_SELECTION_H

#include <vector>

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class RangeSelection : public Operator {
  public:
   struct Range {
      uint32_t start;
      uint32_t end;

      explicit Range(uint32_t start, uint32_t end);
   };

  private:
   std::vector<Range> ranges;
   uint32_t row_count;

  public:
   explicit RangeSelection(std::vector<Range>&& ranges, uint32_t row_count);

   ~RangeSelection() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_RANGE_SELECTION_H
