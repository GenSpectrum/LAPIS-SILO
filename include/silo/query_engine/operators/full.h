#ifndef SILO_FULL_H
#define SILO_FULL_H

#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class Full : public Operator {
   uint32_t sequenceCount;

  public:
   explicit Full(uint32_t sequenceCount);

   ~Full() noexcept override;

   [[nodiscard]] Type type() const override;

   OperatorResult evaluate() const override;

   virtual std::string toString() const override;
};

}  // namespace silo::query_engine::operators

#endif  // SILO_FULL_H
