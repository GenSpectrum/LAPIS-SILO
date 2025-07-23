#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class IntColumnPartition {
  public:
   using Metadata = ColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::INT32;

   static int32_t null() { return INT32_MIN; }

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      // clang-format on
   }

   std::vector<int32_t> values;
   Metadata* metadata;

  public:
   explicit IntColumnPartition(Metadata* metadata);

   [[nodiscard]] const std::vector<int32_t>& getValues() const;

   void insert(int32_t value);

   void insertNull();

   void reserve(size_t row_count);
};

}  // namespace silo::storage::column
