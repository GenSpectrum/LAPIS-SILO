#pragma once

#include <cstdbool>
#include <deque>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <roaring/roaring.hh>

#include "silo/schema/database_schema.h"

namespace silo::storage::column {

class BoolColumnPartition {
  public:
   using Metadata = ColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::BOOL;
   using value_type = bool;

  public:
   roaring::Roaring true_bitmap;
   roaring::Roaring false_bitmap;
   roaring::Roaring null_bitmap;

   Metadata* metadata;

  private:
   size_t num_values = 0;

  public:
   explicit BoolColumnPartition(Metadata* metadata);

   size_t numValues() const { return num_values; }

   [[nodiscard]] bool getValue(size_t row_id) const {
      SILO_ASSERT(!null_bitmap.contains(row_id));
      if (true_bitmap.contains(row_id)) {
         return true;
      }
      return false;
   }

   [[nodiscard]] bool isNull(size_t row_id) const { return null_bitmap.contains(row_id); }

   void insert(bool value);

   void insertNull();

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

}  // namespace silo::storage::column
