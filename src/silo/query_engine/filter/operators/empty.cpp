#include "silo/query_engine/filter/operators/empty.h"

#include <string>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

Empty::Empty(uint32_t row_count)
    : row_count(row_count){};

Empty::~Empty() noexcept = default;

std::string Empty::toString() const {
   return "Empty";
}

Type Empty::type() const {
   return EMPTY;
}

CopyOnWriteBitmap Empty::evaluate() const {
   return {};
}

std::unique_ptr<Operator> Empty::negate(std::unique_ptr<Empty>&& empty) {
   return std::make_unique<Full>(empty->row_count);
}

}  // namespace silo::query_engine::filter::operators
