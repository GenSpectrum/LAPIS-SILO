#pragma once

#include <expected>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/common/date32.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/chunked_value_buffer.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class Date32ColumnBuilder;

class Date32Column {
  public:
   using Metadata = ColumnMetadata;
   using Builder = Date32ColumnBuilder;
   /// Dates are parsed during phase-1 extraction so the chunk holds parsed values.
   using Buffer = std::vector<std::optional<common::Date32>>;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::DATE32;
   using value_type = common::Date32;

   [[maybe_unused]] Metadata* metadata;
   roaring::Roaring null_bitmap;

  private:
   ChunkedValueBuffer<common::Date32> values;
   bool is_sorted = true;
   /// Transient ingestion state: the last non-null value appended, used to detect whether values
   /// stay sorted across chunk boundaries. Not serialized.
   std::optional<common::Date32> last_appended_value;

  public:
   explicit Date32Column(Metadata* metadata);

   [[nodiscard]] bool isSorted() const;

   std::expected<void, std::string> appendChunk(const Buffer& buffer);

   /// The per-chunk value buffers. Used by `DateBetween` to binary search a sorted column.
   [[nodiscard]] const ChunkedValueBuffer<common::Date32>& getValueBuffer() const { return values; }

   [[nodiscard]] size_t numValues() const { return values.numValues(); }

   [[nodiscard]] bool isNull(size_t row_id) const { return null_bitmap.contains(row_id); }

   [[nodiscard]] common::Date32 getValue(size_t row_id) const { return values.at(row_id); }

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /*version*/) {
      // clang-format off
      archive & null_bitmap;
      archive & values;
      archive & is_sorted;
      if constexpr (Archive::is_loading::value) {
         if (numValues() > 0) {
            last_appended_value = values.at(numValues() - 1);
         }
      }
      // clang-format on
   }
};

class Date32ColumnBuilder {
   Date32Column::Buffer buffer;

  public:
   void insert(common::Date32 value) { buffer.emplace_back(value); }

   void insertNull() { buffer.emplace_back(std::nullopt); }

   [[nodiscard]] size_t numValues() const { return buffer.size(); }

   [[nodiscard]] Date32Column::Buffer finalize() {
      Date32Column::Buffer result = std::move(buffer);
      buffer.clear();
      return result;
   }
};

}  // namespace silo::storage::column
