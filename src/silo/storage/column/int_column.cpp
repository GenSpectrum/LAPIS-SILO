#include "silo/storage/column/int_column.h"

#include <spdlog/spdlog.h>

namespace silo::storage::column {

IntColumnPartition::IntColumnPartition(ColumnMetadata* metadata)
    : metadata(metadata) {}

void IntColumnPartition::insert(int32_t value) {
   values.push_back(value);
}

void IntColumnPartition::insertNull() {
   null_bitmap.add(values.size());
   values.push_back(0);
}

}  // namespace silo::storage::column
