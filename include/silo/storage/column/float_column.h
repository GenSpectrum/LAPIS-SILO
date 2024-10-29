#pragma once

#include <cmath>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>

namespace silo::storage::column {

class FloatColumnPartition {
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
   std::vector<double> values;

  public:
   explicit FloatColumnPartition(std::string column_name);

   [[nodiscard]] const std::vector<double>& getValues() const;

   void insert(const std::string& value);

   void insertNull();

   void reserve(size_t row_count);
};

class FloatColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & column_name;
      // clang-format on
   }

   std::string column_name;
   std::deque<FloatColumnPartition> partitions;

  public:
   static double null() { return std::nan(""); }

   explicit FloatColumn(std::string column_name);

   FloatColumnPartition& createPartition();
};

}  // namespace silo::storage::column
