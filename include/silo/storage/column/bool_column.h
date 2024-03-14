#pragma once

#include <cstdbool>
#include <deque>
#include <string>
#include <vector>

#include "silo/common/optional_bool.h"

namespace boost::serialization {
struct access;
}

namespace silo::storage::column {

class BoolColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      // clang-format on
   }

   std::vector<silo::common::OptionalBool> values;

  public:
   BoolColumnPartition();

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
      // clang-format on
   }

   std::deque<BoolColumnPartition> partitions;

  public:
   BoolColumn();

   BoolColumnPartition& createPartition();
};

}  // namespace silo::storage::column
