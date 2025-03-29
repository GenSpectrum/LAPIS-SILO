#include "silo/query_engine/filter/operators/index_scan.h"

#include <string>

#include <fmt/format.h>

#include "external/roaring_include_wrapper.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"

namespace silo::query_engine::filter::operators {

IndexScan::IndexScan(const roaring::Roaring* bitmap, uint32_t row_count)
    : bitmap(bitmap),
      row_count(row_count) {}

IndexScan::IndexScan(
   std::unique_ptr<query_engine::filter::expressions::Expression>&& logical_equivalent,
   const roaring::Roaring* bitmap,
   uint32_t row_count
)
    : logical_equivalent(std::move(logical_equivalent)),
      bitmap(bitmap),
      row_count(row_count) {}

IndexScan::~IndexScan() noexcept = default;

std::string IndexScan::toString() const {
   return fmt::format(
      "IndexScan(Logical Equivalent: {}, Cardinality: {})",
      logical_equivalent ? logical_equivalent.value()->toString() : "undefined",
      std::to_string(bitmap->cardinality())
   );
}

Type IndexScan::type() const {
   return INDEX_SCAN;
}

CopyOnWriteBitmap IndexScan::evaluate() const {
   return CopyOnWriteBitmap(*bitmap);
}
std::unique_ptr<Operator> IndexScan::negate(std::unique_ptr<IndexScan>&& index_scan) {
   const uint32_t row_count = index_scan->row_count;
   return std::make_unique<Complement>(std::move(index_scan), row_count);
}

}  // namespace silo::query_engine::filter::operators
