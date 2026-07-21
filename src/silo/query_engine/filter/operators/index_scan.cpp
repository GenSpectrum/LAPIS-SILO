#include "silo/query_engine/filter/operators/index_scan.h"

#include <string>

#include <fmt/format.h>
#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/operators/complement.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/scalar_expressions/scalar_expression.h"

namespace silo::query_engine::filter::operators {

IndexScan::IndexScan(CopyOnWriteBitmap bitmap, storage::column::RowLayout row_layout)
    : bitmap(std::move(bitmap)),
      row_layout(std::move(row_layout)) {}

IndexScan::IndexScan(
   std::unique_ptr<scalar_expressions::ScalarExpression>&& logical_equivalent,
   CopyOnWriteBitmap bitmap,
   storage::column::RowLayout row_layout
)
    : logical_equivalent(std::move(logical_equivalent)),
      bitmap(std::move(bitmap)),
      row_layout(std::move(row_layout)) {}

IndexScan::~IndexScan() noexcept = default;

std::string IndexScan::toString() const {
   return fmt::format(
      "IndexScan(Logical Equivalent: {}, Cardinality: {})",
      logical_equivalent ? logical_equivalent.value()->toString() : "undefined",
      std::to_string(bitmap.getConstReference().cardinality())
   );
}

Type IndexScan::type() const {
   return INDEX_SCAN;
}

CopyOnWriteBitmap IndexScan::evaluate() const {
   EVOBENCH_SCOPE("IndexScan", "evaluate");
   return bitmap;
}
std::unique_ptr<Operator> IndexScan::negate(std::unique_ptr<IndexScan>&& index_scan) {
   auto row_layout = index_scan->row_layout;
   return std::make_unique<Complement>(std::move(index_scan), std::move(row_layout));
}

}  // namespace silo::query_engine::filter::operators
