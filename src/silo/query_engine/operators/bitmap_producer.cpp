#include "silo/query_engine/filter/operators/bitmap_producer.h"

#include <utility>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

BitmapProducer::BitmapProducer(std::function<CopyOnWriteBitmap()> producer, uint32_t row_count)
    : producer(std::move(producer)),
      row_count(row_count) {}

BitmapProducer::~BitmapProducer() noexcept = default;

std::string BitmapProducer::toString() const {
   return "BitmapProducer";
}

Type BitmapProducer::type() const {
   return BITMAP_PRODUCER;
}

CopyOnWriteBitmap BitmapProducer::evaluate() const {
   return producer();
}

std::unique_ptr<Operator> BitmapProducer::negate(std::unique_ptr<BitmapProducer>&& bitmap_producer
) {
   return std::make_unique<Complement>(std::move(bitmap_producer), bitmap_producer->row_count);
}

}  // namespace silo::query_engine::filter::operators