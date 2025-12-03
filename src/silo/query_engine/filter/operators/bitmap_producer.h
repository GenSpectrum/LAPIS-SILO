#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

class BitmapProducer : public Operator {
  private:
   std::function<CopyOnWriteBitmap()> producer;
   uint32_t row_count;

  public:
   explicit BitmapProducer(std::function<CopyOnWriteBitmap()> producer, uint32_t row_count);

   ~BitmapProducer() noexcept override;

   [[nodiscard]] Type type() const override;

   [[nodiscard]] CopyOnWriteBitmap evaluate() const override;

   [[nodiscard]] std::string toString() const override;

   static std::unique_ptr<Operator> negate(std::unique_ptr<BitmapProducer>&& bitmap_producer);
};

}  // namespace silo::query_engine::filter::operators
