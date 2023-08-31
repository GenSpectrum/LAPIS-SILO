#pragma once

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

namespace boost::serialization {
struct access;
}

namespace silo::storage::column {

class FloatColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & values;
      // clang-format on
   }

   std::vector<double> values;

  public:
   FloatColumnPartition();

   [[nodiscard]] const std::vector<double>& getValues() const;

   void insert(const std::string& value);
};

class FloatColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      // clang-format on
   }

   std::deque<FloatColumnPartition> partitions;

  public:
   FloatColumn();

   FloatColumnPartition& createPartition();
};

}  // namespace silo::storage::column
