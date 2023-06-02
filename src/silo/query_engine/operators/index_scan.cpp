#include "silo/query_engine/operators/index_scan.h"

#include "silo/query_engine/operators/complement.h"

namespace silo::query_engine::operators {

IndexScan::IndexScan(const roaring::Roaring* bitmap, uint32_t row_count)
    : bitmap(bitmap),
      row_count(row_count) {}

IndexScan::~IndexScan() noexcept = default;

std::string IndexScan::toString() const {
   // TODO(someone) think about passing strings for debug printing?
   return "IndexScan(Cardinality: " + std::to_string(bitmap->cardinality()) + ")";
}

Type IndexScan::type() const {
   return INDEX_SCAN;
}

OperatorResult IndexScan::evaluate() const {
   return OperatorResult(bitmap);
}

std::unique_ptr<Operator> IndexScan::copy() const {
   return std::make_unique<IndexScan>(bitmap, row_count);
}

std::unique_ptr<Operator> IndexScan::negate() const {
   return std::make_unique<Complement>(std::make_unique<IndexScan>(bitmap, row_count), row_count);
}

}  // namespace silo::query_engine::operators
