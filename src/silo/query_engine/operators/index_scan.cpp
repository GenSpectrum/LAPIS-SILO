#include "silo/query_engine/operators/index_scan.h"

namespace silo::query_engine::operators {

IndexScan::IndexScan(const roaring::Roaring* bitmap) {
   this->bitmap = OperatorResult(bitmap);
}

IndexScan::~IndexScan() noexcept = default;

std::string IndexScan::toString() const {
   // TODO think about passing strings for debug printing?
   return "IndexScan(Cardinality: " + std::to_string(bitmap.getConst()->cardinality()) + ")";
}

Type IndexScan::type() const {
   return INDEX_SCAN;
}

OperatorResult IndexScan::evaluate() const {
   return bitmap;
}

}  // namespace silo::query_engine::operators
