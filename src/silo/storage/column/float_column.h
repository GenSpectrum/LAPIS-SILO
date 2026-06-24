#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/chunked_value_buffer.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class FloatColumnBuilder;

class FloatColumn {
  public:
   using Metadata = ColumnMetadata;
   using Builder = FloatColumnBuilder;
   using Buffer = std::vector<std::optional<double>>;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::FLOAT;
   using value_type = double;

  private:
   ChunkedValueBuffer<double> values;

  public:
   roaring::Roaring null_bitmap;

   [[maybe_unused]] Metadata* metadata;

   explicit FloatColumn(ColumnMetadata* metadata);

   [[nodiscard]] size_t numChunks() const { return values.numChunks(); }

   [[nodiscard]] uint32_t chunkSize(uint16_t chunk_id) const { return values.chunkSize(chunk_id); }

   [[nodiscard]] bool isNull(RowId row_id) const { return null_bitmap.contains(row_id.toGlobal()); }

   [[nodiscard]] double getValue(RowId row_id) const { return values.at(row_id); }

   std::expected<void, std::string> appendChunk(const Buffer& buffer);

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

class FloatColumnBuilder {
   FloatColumn::Buffer buffer;

  public:
   void insert(double value) { buffer.emplace_back(value); }

   void insertNull() { buffer.emplace_back(std::nullopt); }

   [[nodiscard]] size_t numValues() const { return buffer.size(); }

   [[nodiscard]] FloatColumn::Buffer finalize() {
      FloatColumn::Buffer result = std::move(buffer);
      buffer.clear();
      return result;
   }
};

}  // namespace silo::storage::column
