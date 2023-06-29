#include "silo/storage/column/float_column.h"

namespace silo::storage::column {

FloatColumnPartition::FloatColumnPartition() = default;

const std::vector<double>& FloatColumnPartition::getValues() const {
   return values;
}

void FloatColumnPartition::insert(double value) {
   values.push_back(value);
}

FloatColumn::FloatColumn() = default;

FloatColumnPartition& FloatColumn::createPartition() {
   return partitions.emplace_back();
}

}  // namespace silo::storage::column
