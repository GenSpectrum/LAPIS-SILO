#pragma once

#include <cstdbool>
#include <deque>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/optional_bool.h"
#include "silo/schema/database_schema.h"

namespace silo::storage::column {

class BoolColumnPartition {
  public:
   using Metadata = ColumnMetadata;

   static constexpr schema::ColumnType TYPE = schema::ColumnType::BOOL;
   using value_type = bool;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      // clang-format on
   }

   std::vector<common::OptionalBool> values;

  public:
   Metadata* metadata;
   explicit BoolColumnPartition(Metadata* metadata);

   size_t numValues() const { return values.size(); }

   [[nodiscard]] bool getValue(size_t row_id) const { return values.at(row_id).value().value(); }

   [[nodiscard]] bool isNull(size_t row_id) const { return values.at(row_id).isNull(); }

   void insert(bool value);

   void insertNull();

   void reserve(size_t row_count);
};

}  // namespace silo::storage::column
