#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

namespace silo::storage::column {

class IntColumnPartition {
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
   std::vector<int32_t> values;

  public:
   explicit IntColumnPartition(std::string column_name);

   [[nodiscard]] const std::vector<int32_t>& getValues() const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);
};

class IntColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & column_name;
      // clang-format on
   }

   std::string column_name;
   std::deque<IntColumnPartition> partitions;

  public:
   static int32_t null() { return INT32_MIN; }

   explicit IntColumn(std::string column_name);

   IntColumnPartition& createPartition();
};

}  // namespace silo::storage::column
