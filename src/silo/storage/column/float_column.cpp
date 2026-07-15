#include "silo/storage/column/float_column.h"

#include <cstdint>

#include "silo/storage/column/row_id.h"

namespace silo::storage::column {

FloatColumn::FloatColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> FloatColumn::appendChunk(const Buffer& buffer) {
   const uint32_t base = RowId::chunkStart(static_cast<uint16_t>(values.numChunks()));
   std::vector<double> chunk;
   chunk.reserve(buffer.size());
   for (size_t i = 0; i < buffer.size(); ++i) {
      if (buffer[i].has_value()) {
         chunk.push_back(*buffer[i]);
      } else {
         null_bitmap.add(base + static_cast<uint32_t>(i));
         chunk.push_back(0.0);
      }
   }
   values.appendChunk(std::move(chunk));
   return {};
}

void FloatColumn::update(const roaring::Roaring& row_ids, std::optional<double> value) {
   if (value == std::nullopt) {
      null_bitmap |= row_ids;
   } else {
      null_bitmap -= row_ids;
   }
   const double stored_value = value.value_or(0.0);
   for (const uint32_t global_row_id : row_ids) {
      values.setValue(RowId::fromGlobal(global_row_id), stored_value);
   }
}

}  // namespace silo::storage::column
