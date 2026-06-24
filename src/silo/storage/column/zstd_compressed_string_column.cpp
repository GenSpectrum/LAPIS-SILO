#include "silo/storage/column/zstd_compressed_string_column.h"

namespace silo::storage::column {

ZstdCompressedStringColumnMetadata::ZstdCompressedStringColumnMetadata(
   std::string column_name,
   std::string dictionary_string
)
    : ColumnMetadata(std::move(column_name)),
      compressor(std::make_shared<ZstdCDictionary>(dictionary_string, 3)),
      decompressor(std::make_shared<ZstdDDictionary>(dictionary_string)),
      dictionary_string(std::move(dictionary_string)) {}

ZstdCompressedStringColumn::ZstdCompressedStringColumn(
   silo::storage::column::ZstdCompressedStringColumn::Metadata* metadata
)
    : metadata(metadata) {}

std::expected<void, std::string> ZstdCompressedStringColumn::appendChunk(const Buffer& buffer) {
   const size_t base = numValues();
   std::vector<std::string> chunk;
   chunk.reserve(buffer.size());
   for (size_t i = 0; i < buffer.size(); ++i) {
      const auto& value = buffer[i];
      if (value.has_value()) {
         chunk.emplace_back(metadata->compressor.compress(value->data(), value->size()));
      } else {
         null_bitmap.add(base + i);
         chunk.emplace_back();
      }
   }
   values.appendChunk(std::move(chunk));
   return {};
}

bool ZstdCompressedStringColumn::isNull(size_t row_id) const {
   return null_bitmap.contains(row_id);
}

std::optional<std::string> ZstdCompressedStringColumn::getDecompressed(size_t row_id) const {
   const auto value = values.at(row_id);
   if (value.empty()) {
      return std::nullopt;
   }
   std::string result_buffer;
   metadata->decompressor.decompress(value, result_buffer);
   return result_buffer;
}

std::optional<std::string> ZstdCompressedStringColumn::getCompressed(size_t row_id) const {
   auto value = values.at(row_id);
   if (value.empty()) {
      return std::nullopt;
   }
   return value;
}

}  // namespace silo::storage::column
