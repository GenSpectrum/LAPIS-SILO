#include "silo/storage/column/int_column.h"

#include <spdlog/spdlog.h>

namespace silo::storage::column {

IntColumn::IntColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> IntColumn::appendChunk(const Buffer& buffer) {
   values.reserve(values.size() + buffer.size());
   for (const auto& value : buffer) {
      if (value.has_value()) {
         values.push_back(*value);
      } else {
         null_bitmap.add(values.size());
         values.push_back(0);
      }
   }
   return {};
}

}  // namespace silo::storage::column
