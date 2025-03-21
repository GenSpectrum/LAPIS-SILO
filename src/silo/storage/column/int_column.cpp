#include "silo/storage/column/int_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column {

IntColumnPartition::IntColumnPartition(ColumnMetadata* metadata)
    : metadata(metadata) {}

const std::vector<int32_t>& IntColumnPartition::getValues() const {
   return values;
}

void IntColumnPartition::insert(int32_t value) {
   values.push_back(value);
}

void IntColumnPartition::insertNull() {
   values.push_back(IntColumnPartition::null());
}

void IntColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

}  // namespace silo::storage::column
