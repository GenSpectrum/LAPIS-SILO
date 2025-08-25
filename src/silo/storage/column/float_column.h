#pragma once

#include <cmath>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_metadata.h"

namespace silo::storage::column {

class FloatColumnPartition {
  public:
   using Metadata = ColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::FLOAT;
   using value_type = double;

   static double null() { return std::nan(""); }

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      // clang-format on
   }

   std::vector<double> values;

  public:
   [[maybe_unused]] Metadata* metadata;

   explicit FloatColumnPartition(ColumnMetadata* metadata);

   size_t numValues() const { return values.size(); }

   [[nodiscard]] bool isNull(size_t row_id) const { return values.at(row_id) == null(); }

   double getValue(size_t row_id) const { return values.at(row_id); }

   void insert(double value);

   void insertNull();

   void reserve(size_t row_count);
};

}  // namespace silo::storage::column
