#include "silo/query_engine/filter/operators/bitmap_producer.h"

#include <utility>

#include "evobench/evobench.hpp"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

BitmapProducer::BitmapProducer(
   std::function<CopyOnWriteBitmap()> producer,
   storage::column::RowLayout row_layout
)
    : producer(std::move(producer)),
      row_layout(std::move(row_layout)) {}

BitmapProducer::~BitmapProducer() noexcept = default;

std::string BitmapProducer::toString() const {
   return "BitmapProducer";
}

Type BitmapProducer::type() const {
   return BITMAP_PRODUCER;
}

CopyOnWriteBitmap BitmapProducer::evaluate() const {
   EVOBENCH_SCOPE("BitmapProducer", "evaluate");
   return producer();
}

std::unique_ptr<Operator> BitmapProducer::negate(std::unique_ptr<BitmapProducer>&& bitmap_producer
) {
   auto row_layout = bitmap_producer->row_layout;
   return std::make_unique<Complement>(std::move(bitmap_producer), std::move(row_layout));
}

}  // namespace silo::query_engine::filter::operators
