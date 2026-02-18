#pragma once

#include <cstdint>
#include <expected>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class IntColumnPartition {
  public:
   using Metadata = ColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::INT32;
   using value_type = int32_t;

  private:
   std::vector<int32_t> values;

  public:
   roaring::Roaring null_bitmap;

   Metadata* metadata;

   explicit IntColumnPartition(Metadata* metadata);

   [[nodiscard]] bool isNull(size_t row_id) const { return null_bitmap.contains(row_id); }

   [[nodiscard]] int32_t getValue(size_t row_id) const {
      SILO_ASSERT(!null_bitmap.contains(row_id));
      return values.at(row_id);
   }

   [[nodiscard]] size_t numValues() const { return values.size(); }

   [[nodiscard]] std::expected<void, std::string> insert(int32_t value);

   void insertNull();

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      archive & null_bitmap;
      // clang-format on
   }
};

}  // namespace silo::storage::column
