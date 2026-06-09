#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_free.hpp>
#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/zstd/zstd_compressor.h"
#include "silo/zstd/zstd_decompressor.h"
#include "silo/zstd/zstd_dictionary.h"

namespace silo::storage::column {

class ZstdCompressedStringColumnBuilder;

class ZstdCompressedStringColumnMetadata : public ColumnMetadata {
  public:
   ZstdCompressor compressor;
   ZstdDecompressor decompressor;
   std::string dictionary_string;

   explicit ZstdCompressedStringColumnMetadata(
      std::string column_name,
      std::string dictionary_string
   );
};

class ZstdCompressedStringColumn {
  public:
   using Metadata = ZstdCompressedStringColumnMetadata;
   using Builder = ZstdCompressedStringColumnBuilder;
   using Buffer = std::vector<std::optional<std::string>>;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::ZSTD_COMPRESSED_STRING;
   using value_type = std::string_view;

  private:
   std::vector<std::string> values;

  public:
   roaring::Roaring null_bitmap;
   Metadata* metadata;

   explicit ZstdCompressedStringColumn(Metadata* metadata);

   [[nodiscard]] std::expected<void, std::string> appendChunk(const Buffer& buffer);

   [[nodiscard]] bool isNull(size_t row_id) const;

   [[nodiscard]] size_t numValues() const { return values.size(); }

   [[nodiscard]] std::optional<std::string> getDecompressed(size_t row_id) const;

   [[nodiscard]] std::optional<std::string> getCompressed(size_t row_id) const;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /*version*/) {
      // clang-format off
      archive & values;
      archive & null_bitmap;
      // clang-format on
   }
};

class ZstdCompressedStringColumnBuilder {
   ZstdCompressedStringColumn::Buffer buffer;

  public:
   void insert(std::string_view value) { buffer.emplace_back(std::string{value}); }

   void insertNull() { buffer.emplace_back(std::nullopt); }

   [[nodiscard]] size_t numValues() const { return buffer.size(); }

   [[nodiscard]] ZstdCompressedStringColumn::Buffer finalize() {
      ZstdCompressedStringColumn::Buffer result = std::move(buffer);
      buffer.clear();
      return result;
   }
};

}  // namespace silo::storage::column

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::ZstdCompressedStringColumnMetadata);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& archive,
   const silo::storage::column::ZstdCompressedStringColumnMetadata& object,
   [[maybe_unused]] const uint32_t version
) {
   archive & object.column_name;
   archive & object.dictionary_string;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<
                               silo::storage::column::ZstdCompressedStringColumnMetadata>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& archive,
   std::shared_ptr<silo::storage::column::ZstdCompressedStringColumnMetadata>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   std::string dictionary_string;
   archive & column_name;
   archive & dictionary_string;
   object = std::make_shared<silo::storage::column::ZstdCompressedStringColumnMetadata>(
      std::move(column_name), std::move(dictionary_string)
   );
}
}  // namespace boost::serialization
