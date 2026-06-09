#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/column.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class IntColumnBuilder;

class IntColumn {
  public:
   using Metadata = ColumnMetadata;
   using Builder = IntColumnBuilder;
   using Buffer = std::vector<std::optional<int32_t>>;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::INT32;
   using value_type = int32_t;

  private:
   std::vector<int32_t> values;

  public:
   roaring::Roaring null_bitmap;

   Metadata* metadata;

   explicit IntColumn(Metadata* metadata);

   [[nodiscard]] bool isNull(size_t row_id) const { return null_bitmap.contains(row_id); }

   [[nodiscard]] int32_t getValue(size_t row_id) const {
      SILO_ASSERT(!null_bitmap.contains(row_id));
      return values.at(row_id);
   }

   [[nodiscard]] size_t numValues() const { return values.size(); }

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

class IntColumnBuilder {
   IntColumn::Buffer buffer;

  public:
   void insert(int32_t value) { buffer.emplace_back(value); }

   void insertNull() { buffer.emplace_back(std::nullopt); }

   [[nodiscard]] size_t numValues() const { return buffer.size(); }

   [[nodiscard]] IntColumn::Buffer finalize() {
      IntColumn::Buffer result = std::move(buffer);
      buffer.clear();
      return result;
   }
};

}  // namespace silo::storage::column
