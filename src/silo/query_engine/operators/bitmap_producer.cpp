#include "silo/query_engine/operators/bitmap_producer.h"

#include "silo/query_engine/operators/complement.h"

namespace silo::query_engine::operators {

BitmapProducer::BitmapProducer(std::function<OperatorResult()> producer, uint32_t row_count)
    : producer(std::move(producer)),
      row_count(row_count) {}

BitmapProducer::~BitmapProducer() noexcept = default;

std::string BitmapProducer::toString() const {
   return "BitmapProducer";
}

Type BitmapProducer::type() const {
   return BITMAP_PRODUCER;
}

OperatorResult BitmapProducer::evaluate() const {
   return producer();
}

std::unique_ptr<Operator> BitmapProducer::copy() const {
   return std::make_unique<BitmapProducer>(producer, row_count);
}

std::unique_ptr<Operator> BitmapProducer::negate() const {
   return std::make_unique<Complement>(copy(), row_count);
}

}  // namespace silo::query_engine::operators