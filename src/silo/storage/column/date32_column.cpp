#include "silo/storage/column/date32_column.h"

#include "silo/common/date32.h"

namespace silo::storage::column {

Date32Column::Date32Column(ColumnMetadata* metadata)
    : metadata(metadata) {}

bool Date32Column::isSorted() const {
   return is_sorted;
}

std::expected<void, std::string> Date32Column::appendChunk(const Buffer& buffer) {
   const size_t base = numValues();
   std::vector<common::Date32> chunk;
   chunk.reserve(buffer.size());
   for (size_t i = 0; i < buffer.size(); ++i) {
      if (buffer[i].has_value()) {
         if (last_appended_value.has_value() && *buffer[i] < *last_appended_value) {
            is_sorted = false;
         }
         last_appended_value = buffer[i];
         chunk.push_back(*buffer[i]);
      } else {
         null_bitmap.add(base + i);
         chunk.push_back(0);
         is_sorted = false;
      }
   }
   values.appendChunk(std::move(chunk));
   return {};
}

}  // namespace silo::storage::column
