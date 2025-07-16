#include "silo/storage/column/bool_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

namespace silo::storage::column {

BoolColumnPartition::BoolColumnPartition(ColumnMetadata* metadata)
    : metadata(metadata) {}

void BoolColumnPartition::insert(bool value) {
   values.emplace_back(value);
}

void BoolColumnPartition::insertNull() {
   values.emplace_back();
}

void BoolColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

}  // namespace silo::storage::column
