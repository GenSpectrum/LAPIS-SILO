#pragma once

#include <cstdint>
#include <deque>
#include <filesystem>
#include <string>

#include <boost/serialization/access.hpp>

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

   YAML::Node toYAML() const override;
   static std::shared_ptr<ZstdCompressedStringColumnMetadata> fromYAML(
      std::string column_name,
      const YAML::Node& node
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
