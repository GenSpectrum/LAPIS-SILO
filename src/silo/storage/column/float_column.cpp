#include "silo/storage/column/float_column.h"

#include <cmath>

namespace silo::storage::column {

FloatColumn::FloatColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> FloatColumn::insert(double value) {
   values.push_back(value);
   return {};
}

void FloatColumn::insertNull() {
   null_bitmap.add(values.size());
   values.push_back(std::nan(""));
}

void FloatColumn::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

}  // namespace silo::storage::column
