#include "silo/query_engine/operators/full.h"

#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/operators/empty.h"
#include "silo/query_engine/operators/operator.h"

namespace silo::query_engine::operators {

Full::Full(uint32_t row_count)
    : row_count(row_count) {}

Full::~Full() noexcept = default;

std::string Full::toString() const {
   return "Full";
}

Type Full::type() const {
   return FULL;
}

CopyOnWriteBitmap Full::evaluate() const {
   CopyOnWriteBitmap result;
   result->addRange(0, row_count);
   return result;
}

std::unique_ptr<Operator> Full::negate(std::unique_ptr<Full>&& full) {
   return std::make_unique<Empty>(full->row_count);
}

}  // namespace silo::query_engine::operators
