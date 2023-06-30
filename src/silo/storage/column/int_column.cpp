#include "silo/storage/column/int_column.h"

namespace silo::storage::column {

IntColumnPartition::IntColumnPartition() = default;

const std::vector<int64_t>& IntColumnPartition::getValues() const {
   return values;
}

void IntColumnPartition::insert(int64_t value) {
   values.push_back(value);
}

IntColumn::IntColumn() = default;

IntColumnPartition& IntColumn::createPartition() {
   return partitions.emplace_back();
}

}  // namespace silo::storage::column
