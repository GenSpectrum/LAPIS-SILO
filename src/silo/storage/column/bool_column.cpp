#include "silo/storage/column/bool_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

namespace silo::storage::column {

BoolColumn::BoolColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> BoolColumn::appendChunk(const Buffer& buffer) {
   for (const auto& value : buffer) {
      if (!value.has_value()) {
         null_bitmap.add(num_values++);
      } else if (*value) {
         true_bitmap.add(num_values++);
      } else {
         false_bitmap.add(num_values++);
      }
   }
   return {};
}
}  // namespace silo::storage::column
