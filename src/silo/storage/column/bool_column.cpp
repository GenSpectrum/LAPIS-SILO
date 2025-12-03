#include "silo/storage/column/bool_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

namespace silo::storage::column {

BoolColumnPartition::BoolColumnPartition(ColumnMetadata* metadata)
    : metadata(metadata) {}

void BoolColumnPartition::insert(bool value) {
   if (value) {
      true_bitmap.add(num_values++);
   } else {
      false_bitmap.add(num_values++);
   }
}

void BoolColumnPartition::insertNull() {
   null_bitmap.add(num_values++);
}
}  // namespace silo::storage::column
