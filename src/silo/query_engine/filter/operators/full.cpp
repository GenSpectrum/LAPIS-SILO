#include "silo/query_engine/filter/operators/full.h"

#include <string>
#include <utility>

#include "evobench/evobench.hpp"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/empty.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

Full::Full(storage::column::RowLayout row_layout)
    : row_layout(std::move(row_layout)) {}

Full::~Full() noexcept = default;

std::string Full::toString() const {
   return "Full";
}

Type Full::type() const {
   return FULL;
}

CopyOnWriteBitmap Full::evaluate() const {
   EVOBENCH_SCOPE("Full", "evaluate");
   return CopyOnWriteBitmap{row_layout.fullBitmap()};
}

std::unique_ptr<Operator> Full::negate(std::unique_ptr<Full>&& full) {
   return std::make_unique<Empty>(std::move(full->row_layout));
}

}  // namespace silo::query_engine::filter::operators
