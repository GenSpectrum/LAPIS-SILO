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

   std::vector<int32_t> values;

  public:
   BoolColumnPartition();

   [[nodiscard]] const std::vector<OptionalBool>& getValues() const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);
};

class BoolColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const ubool32_t /* version */) {
      // clang-format off
      // clang-format on
   }

   std::deque<BoolColumnPartition> partitions;

  public:
   BoolColumn();

   BoolColumnPartition& createPartition();
};

}  // namespace silo::storage::column
