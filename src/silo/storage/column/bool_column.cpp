#include "silo/storage/column/bool_column.h"

#include <stdexcept>

#include <spdlog/spdlog.h>

#include "silo/storage/column/row_id.h"

namespace silo::storage::column {

BoolColumn::BoolColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> BoolColumn::appendChunk(const Buffer& buffer) {
   const uint32_t base = RowId::chunkStart(num_chunks);
   for (size_t i = 0; i < buffer.size(); ++i) {
      const uint32_t row_id = base + static_cast<uint32_t>(i);
      const auto& value = buffer[i];
      if (!value.has_value()) {
         null_bitmap.add(row_id);
      } else if (*value) {
         true_bitmap.add(row_id);
      } else {
         false_bitmap.add(row_id);
      }
   }
   num_chunks++;
   return {};
}

void BoolColumn::update(const roaring::Roaring& row_ids, std::optional<bool> value) {
   if (value == std::nullopt) {
      null_bitmap |= row_ids;
   } else {
      null_bitmap -= row_ids;
   }
   if (value == true) {
      true_bitmap |= row_ids;
   } else {
      true_bitmap -= row_ids;
   }
   if (value == false) {
      false_bitmap |= row_ids;
   } else {
      false_bitmap -= row_ids;
   }
}
}  // namespace silo::storage::column
