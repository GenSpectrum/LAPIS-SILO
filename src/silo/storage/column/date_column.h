#pragma once

#include <cstdint>
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
   using value_type = common::Date;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      archive & is_sorted;
      // clang-format on
   }

   std::vector<silo::common::Date> values;
   bool is_sorted;

  public:
   [[maybe_unused]] Metadata* metadata;

   explicit DateColumnPartition(Metadata* metadata);

   [[nodiscard]] bool isSorted() const;

   void insert(std::string_view value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] const std::vector<silo::common::Date>& getValues() const;

   [[nodiscard]] size_t numValues() const { return values.size(); }

   [[nodiscard]] bool isNull(size_t row_id) const { return values.at(row_id) == common::NULL_DATE; }
   [[nodiscard]] silo::common::Date getValue(size_t row_id) const { return values.at(row_id); }
};

}  // namespace silo::storage::column
