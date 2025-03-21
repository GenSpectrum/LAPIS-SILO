#include "silo/storage/column/float_column.h"

#include <cmath>
#include <stdexcept>

#include <fmt/format.h>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column {

FloatColumnPartition::FloatColumnPartition(ColumnMetadata* metadata)
    : metadata(metadata) {}

const std::vector<double>& FloatColumnPartition::getValues() const {
   return values;
}

void FloatColumnPartition::insert(double value) {
   values.push_back(value);
}

void FloatColumnPartition::insertNull() {
   values.push_back(std::nan(""));
}

void FloatColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

}  // namespace silo::storage::column
