#pragma once

#include <cmath>
#include <cstdint>
#include <expected>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class FloatColumnPartition {
  public:
   using Metadata = ColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::FLOAT;
   using value_type = double;

  private:
   std::vector<double> values;

  public:
   roaring::Roaring null_bitmap;

   [[maybe_unused]] Metadata* metadata;

   explicit FloatColumnPartition(ColumnMetadata* metadata);

   [[nodiscard]] size_t numValues() const { return values.size(); }

   [[nodiscard]] bool isNull(size_t row_id) const { return null_bitmap.contains(row_id); }

   [[nodiscard]] double getValue(size_t row_id) const { return values.at(row_id); }

   [[nodiscard]] std::expected<void, std::string> insert(double value);

   void insertNull();

   void reserve(size_t row_count);

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
