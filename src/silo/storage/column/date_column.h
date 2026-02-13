#pragma once

#include <cstdint>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/common/date.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class DateColumnPartition {
  public:
   using Metadata = ColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::DATE;
   using value_type = common::Date;

   [[maybe_unused]] Metadata* metadata;
   roaring::Roaring null_bitmap;

  private:
   std::vector<silo::common::Date> values;
   bool is_sorted = true;

  public:
   explicit DateColumnPartition(Metadata* metadata);

   [[nodiscard]] bool isSorted() const;

   void insert(std::string_view value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] const std::vector<silo::common::Date>& getValues() const;

   [[nodiscard]] size_t numValues() const { return values.size(); }

   [[nodiscard]] bool isNull(size_t row_id) const { return null_bitmap.contains(row_id); }
   [[nodiscard]] silo::common::Date getValue(size_t row_id) const { return values.at(row_id); }

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & null_bitmap;
      archive & values;
      archive & is_sorted;
      // clang-format on
   }
};

}  // namespace silo::storage::column
