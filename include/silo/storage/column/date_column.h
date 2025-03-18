#pragma once

#include <cstdint>
#include <deque>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/date.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class DateColumnPartition {
  public:
   using Metadata = ColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::DATE;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      archive & is_sorted;
      // clang-format on
   }

   [[maybe_unused]] Metadata* metadata;
   std::vector<silo::common::Date> values;
   bool is_sorted;

  public:
   explicit DateColumnPartition(Metadata* metadata);

   [[nodiscard]] bool isSorted() const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] const std::vector<silo::common::Date>& getValues() const;
};

}  // namespace silo::storage::column
