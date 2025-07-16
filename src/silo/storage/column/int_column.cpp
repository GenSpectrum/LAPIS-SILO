#include "silo/storage/column/int_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column {

IntColumnPartition::IntColumnPartition(ColumnMetadata* metadata)
    : metadata(metadata) {}

void IntColumnPartition::insert(int32_t value) {
   values.push_back(value);
}

void IntColumnPartition::insertNull() {
   values.push_back(IntColumnPartition::null());
}

}  // namespace silo::storage::column
