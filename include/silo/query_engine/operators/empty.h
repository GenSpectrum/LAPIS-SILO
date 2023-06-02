#ifndef SILO_EMPTY_H
#define SILO_EMPTY_H

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class Empty : public Operator {
  private:
   uint32_t row_count;

  public:
   explicit Empty(uint32_t row_count);

   ~Empty() noexcept override;

   [[nodiscard]] Type type() const override;

   OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_EMPTY_H
