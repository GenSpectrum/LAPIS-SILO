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
   using value_type = int32_t;

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

  public:
   Metadata* metadata;

   explicit IntColumnPartition(Metadata* metadata);

   [[nodiscard]] bool isNull(size_t row_id) const { return values.at(row_id) == null(); }

   [[nodiscard]] int32_t getValue(size_t row_id) const { return values.at(row_id); }

   size_t numValues() const { return values.size(); }

   void insert(int32_t value);

   void insertNull();
};

}  // namespace silo::storage::column
