#pragma once

#include <expected>
#include <optional>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class BoolColumnBuilder;

class BoolColumn {
  public:
   using Metadata = ColumnMetadata;
   using Builder = BoolColumnBuilder;
   using Buffer = std::vector<std::optional<bool>>;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::BOOL;
   using value_type = bool;

   roaring::Roaring true_bitmap;
   roaring::Roaring false_bitmap;
   roaring::Roaring null_bitmap;

   Metadata* metadata;

  private:
   size_t num_values = 0;

  public:
   explicit BoolColumn(Metadata* metadata);

   [[nodiscard]] size_t numValues() const { return num_values; }

   [[nodiscard]] bool getValue(size_t row_id) const {
      SILO_ASSERT(!null_bitmap.contains(row_id));
      return true_bitmap.contains(row_id);
   }

   [[nodiscard]] bool isNull(size_t row_id) const { return null_bitmap.contains(row_id); }

   std::expected<void, std::string> appendChunk(const Buffer& buffer);

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & true_bitmap;
      archive & false_bitmap;
      archive & null_bitmap;
      archive & num_values;
      // clang-format on
   }
};

class BoolColumnBuilder {
   BoolColumn::Buffer buffer;

  public:
   void insert(bool value) { buffer.emplace_back(value); }

   void insertNull() { buffer.emplace_back(std::nullopt); }

   [[nodiscard]] size_t numValues() const { return buffer.size(); }

   [[nodiscard]] BoolColumn::Buffer finalize() {
      BoolColumn::Buffer result = std::move(buffer);
      buffer.clear();
      return result;
   }
};

}  // namespace silo::storage::column
