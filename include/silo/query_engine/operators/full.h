#pragma once

#include <cstdint>
#include <memory>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class Full : public Operator {
   uint32_t row_count;

  public:
   explicit Full(uint32_t row_count);

   ~Full() noexcept override;

   [[nodiscard]] Type type() const override;

   OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   virtual std::unique_ptr<Operator> copy() const override;

   virtual std::unique_ptr<Operator> negate() const override;
};

}  // namespace silo::query_engine::operators
