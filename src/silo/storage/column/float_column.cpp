#include "silo/storage/column/float_column.h"

#include <cmath>

namespace silo::storage::column {

FloatColumn::FloatColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> FloatColumn::appendChunk(const Buffer& buffer) {
   values.reserve(values.size() + buffer.size());
   for (const auto& value : buffer) {
      if (value.has_value()) {
         values.push_back(*value);
      } else {
         null_bitmap.add(values.size());
         values.push_back(std::nan(""));
      }
   }
   return {};
}

}  // namespace silo::storage::column
