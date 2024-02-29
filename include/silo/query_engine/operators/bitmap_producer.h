#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

class BitmapProducer : public Operator {
  private:
   std::function<OperatorResult()> producer;
   uint32_t row_count;

  public:
   explicit BitmapProducer(std::function<OperatorResult()> producer, uint32_t row_count);

   ~BitmapProducer() noexcept override;

   [[nodiscard]] virtual Type type() const override;

   virtual OperatorResult evaluate() const override;

   virtual std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<BitmapProducer>&& bitmap_producer);
};

}  // namespace silo::query_engine::operators
