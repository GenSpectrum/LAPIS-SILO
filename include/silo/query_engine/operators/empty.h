#ifndef SILO_EMPTY_H
#define SILO_EMPTY_H

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class Empty : public Operator {
  public:
   explicit Empty();

   ~Empty() noexcept override;

   [[nodiscard]] Type type() const override;

   OperatorResult evaluate() const override;

   virtual std::string toString() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_EMPTY_H
