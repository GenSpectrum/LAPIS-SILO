#include "silo/query_engine/filter/operators/empty.h"

#include <string>
#include <utility>

#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/full.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

Empty::Empty(storage::column::RowLayout row_layout)
    : row_layout(std::move(row_layout)) {};

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
   return std::make_unique<Full>(std::move(empty->row_layout));
}

}  // namespace silo::query_engine::filter::operators
