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

ZstdCompressedStringColumnPartition::ZstdCompressedStringColumnPartition(
   silo::storage::column::ZstdCompressedStringColumnPartition::Metadata* metadata
)
    : metadata(metadata) {}

void ZstdCompressedStringColumnPartition::reserve(size_t row_count) {
   values.reserve(row_count);
}

void ZstdCompressedStringColumnPartition::insertNull() {
   values.push_back({});
}

void ZstdCompressedStringColumnPartition::insert(const std::string& value) {
   auto compressed = metadata->compressor.compress(value);
   values.push_back(std::string{compressed});
}

std::optional<std::string> ZstdCompressedStringColumnPartition::getDecompressed(size_t row_id
) const {
   const auto value = values.at(row_id);
   if (value.empty()) {
      return std::nullopt;
   }
   std::string result_buffer;
   metadata->decompressor.decompress(value, result_buffer);
   return result_buffer;
}

std::optional<std::string> ZstdCompressedStringColumnPartition::getCompressed(size_t row_id) const {
   auto value = values.at(row_id);
   if (value.empty()) {
      return std::nullopt;
   }
   return value;
}

}  // namespace silo::storage::column
