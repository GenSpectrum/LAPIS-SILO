#include "silo/storage/column/float_column.h"

namespace silo::storage::column {

FloatColumn::FloatColumn(ColumnMetadata* metadata)
    : metadata(metadata) {}

std::expected<void, std::string> FloatColumn::appendChunk(const Buffer& buffer) {
   const size_t base = numValues();
   std::vector<double> chunk;
   chunk.reserve(buffer.size());
   for (size_t i = 0; i < buffer.size(); ++i) {
      if (buffer[i].has_value()) {
         chunk.push_back(*buffer[i]);
      } else {
         null_bitmap.add(base + i);
         chunk.push_back(0.0);
      }
   }
   values.appendChunk(std::move(chunk));
   return {};
}

}  // namespace silo::storage::column
