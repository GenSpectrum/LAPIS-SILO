#pragma once

#include <cstdint>
#include <deque>
#include <vector>

#include "silo/common/date.h"

namespace boost::serialization {
class access;
}

namespace silo::storage::column {

class DateColumnPartition {
   friend class boost::serialization::access;

  private:
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
   explicit DateColumnPartition(bool is_sorted);

   [[nodiscard]] bool isSorted() const;

   void insert(const silo::common::Date& value);

   void insertNull();

   void reserve(size_t row_count);

   [[nodiscard]] const std::vector<silo::common::Date>& getValues() const;
};

class DateColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & is_sorted;
      // clang-format on
   }

   bool is_sorted;
   std::deque<DateColumnPartition> partitions;

   DateColumn();

  public:
   explicit DateColumn(bool is_sorted);

   DateColumnPartition& createPartition();
};

}  // namespace silo::storage::column
