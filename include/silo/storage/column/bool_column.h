#pragma once

#include <cstdbool>
#include <deque>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/optional_bool.h"

namespace silo::storage::column {

class BoolColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & column_name;
      archive & values;
      // clang-format on
   }

   std::string column_name;
   std::vector<silo::common::OptionalBool> values;

  public:
   explicit BoolColumnPartition(std::string column_name);

   [[nodiscard]] const std::vector<silo::common::OptionalBool>& getValues() const;

   void insert(bool value);

   void insertNull();

   void reserve(size_t row_count);
};

class BoolColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & column_name;
      // clang-format on
   }

   std::string column_name;
   std::deque<BoolColumnPartition> partitions;

  public:
   explicit BoolColumn(std::string column_name);

   BoolColumnPartition& createPartition();
};

}  // namespace silo::storage::column
