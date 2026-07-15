#include "silo/storage/column/date32_column.h"

#include <cstdint>

#include "silo/common/date32.h"
#include "silo/storage/column/row_id.h"

namespace silo::storage::column {

Date32Column::Date32Column(ColumnMetadata* metadata)
    : metadata(metadata) {}

bool Date32Column::isSorted() const {
   return is_sorted;
}

std::expected<void, std::string> Date32Column::appendChunk(const Buffer& buffer) {
   const uint32_t base = RowId::chunkStart(static_cast<uint16_t>(values.numChunks()));
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
         null_bitmap.add(base + static_cast<uint32_t>(i));
         chunk.push_back(0);
         is_sorted = false;
      }
   }
   values.appendChunk(std::move(chunk));
   return {};
}

void Date32Column::update(const roaring::Roaring& row_ids, std::optional<common::Date32> value) {
   if (value.has_value()) {
      null_bitmap -= row_ids;
   } else {
      null_bitmap |= row_ids;
   }
   const common::Date32 stored_value = value.value_or(0);
   for (const uint32_t global_row_id : row_ids) {
      values.setValue(RowId::fromGlobal(global_row_id), stored_value);
   }

   // An update can move a value in either direction, so the cheap incremental sortedness tracking
   // of `appendChunk` no longer applies; recompute it by scanning the whole column. As in
   // `appendChunk`, any null makes the column unsorted, and `last_appended_value` is the last
   // non-null value so a subsequent `appendChunk` can keep detecting cross-chunk regressions.
   is_sorted = true;
   last_appended_value = std::nullopt;

   if (!null_bitmap.isEmpty()) {
      is_sorted = false;
      return;
   }
   for (size_t chunk_id = 0; chunk_id < values.numChunks(); ++chunk_id) {
      const std::vector<common::Date32>& chunk = values.chunk(chunk_id);
      for (common::Date32 date : chunk) {
         if (last_appended_value.has_value() && date < *last_appended_value) {
            is_sorted = false;
         }
         last_appended_value = date;
      }
   }
}

}  // namespace silo::storage::column
