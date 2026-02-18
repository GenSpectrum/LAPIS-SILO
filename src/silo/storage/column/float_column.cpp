#include "silo/storage/column/float_column.h"

#include <cmath>

namespace silo::storage::column {

FloatColumnPartition::FloatColumnPartition(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> FloatColumnPartition::insert(double value) {
   values.push_back(value);
   return {};
}

void FloatColumnPartition::insertNull() {
   null_bitmap.add(values.size());
   values.push_back(std::nan(""));
}

void FloatColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

}  // namespace silo::storage::column
