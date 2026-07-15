#include "silo/storage/column/int_column.h"

#include <cstdint>

#include "silo/storage/column/row_id.h"

namespace silo::storage::column {

IntColumn::IntColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> IntColumn::appendChunk(const Buffer& buffer) {
   const uint32_t base = RowId::chunkStart(static_cast<uint16_t>(values.numChunks()));
   std::vector<int32_t> chunk;
   chunk.reserve(buffer.size());
   for (size_t i = 0; i < buffer.size(); ++i) {
      if (buffer[i].has_value()) {
         chunk.push_back(*buffer[i]);
      } else {
         null_bitmap.add(base + static_cast<uint32_t>(i));
         chunk.push_back(0);
      }
   }
   values.appendChunk(std::move(chunk));
   return {};
}

void IntColumn::update(const roaring::Roaring& row_ids, std::optional<int32_t> value) {
   if (value == std::nullopt) {
      null_bitmap |= row_ids;
   } else {
      null_bitmap -= row_ids;
   }
   const int32_t stored_value = value.value_or(0);
   for (const uint32_t global_row_id : row_ids) {
      values.setValue(RowId::fromGlobal(global_row_id), stored_value);
   }
}

}  // namespace silo::storage::column
