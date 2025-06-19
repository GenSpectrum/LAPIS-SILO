#pragma once

#include <cstdint>
#include <deque>
#include <filesystem>
#include <string>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_free.hpp>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"
#include "silo/zstd/zstd_compressor.h"
#include "silo/zstd/zstd_decompressor.h"
#include "silo/zstd/zstd_dictionary.h"

namespace silo::storage::column {

class ZstdCompressedStringColumnMetadata : public ColumnMetadata {
  public:
   ZstdCompressor compressor;
   ZstdDecompressor decompressor;
   std::string dictionary_string;

  public:
   explicit ZstdCompressedStringColumnMetadata(
      std::string column_name,
      std::string dictionary_string
   );
};

/// Holds information where to read unaligned sequences for a
/// segment (= the sequence of a particular name) in one partition.
class ZstdCompressedStringColumnPartition {
  public:
   using Metadata = ZstdCompressedStringColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::ZSTD_COMPRESSED_STRING;

  private:
   std::vector<std::string> values;

  public:
   Metadata* metadata;

   explicit ZstdCompressedStringColumnPartition(Metadata* metadata);

   void reserve(size_t row_count);
   void insertNull();
   void insert(const std::string& value);

   std::optional<std::string> getDecompressed(size_t row_id) const;

   std::optional<std::string> getCompressed(size_t row_id) const;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      // clang-format on
   }
};

}  // namespace silo::storage::column

BOOST_SERIALIZATION_SPLIT_FREE(silo::storage::column::ZstdCompressedStringColumnMetadata);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void save(
   Archive& ar,
   const silo::storage::column::ZstdCompressedStringColumnMetadata& object,
   [[maybe_unused]] const uint32_t version
) {
   ar & object.column_name;
   ar & object.dictionary_string;
}
}  // namespace boost::serialization

BOOST_SERIALIZATION_SPLIT_FREE(std::shared_ptr<
                               silo::storage::column::ZstdCompressedStringColumnMetadata>);
namespace boost::serialization {
template <class Archive>
[[maybe_unused]] void load(
   Archive& ar,
   std::shared_ptr<silo::storage::column::ZstdCompressedStringColumnMetadata>& object,
   [[maybe_unused]] const uint32_t version
) {
   std::string column_name;
   std::string dictionary_string;
   ar & column_name;
   ar & dictionary_string;
   object = std::make_shared<silo::storage::column::ZstdCompressedStringColumnMetadata>(
      std::move(column_name), std::move(dictionary_string)
   );
}
}  // namespace boost::serialization
