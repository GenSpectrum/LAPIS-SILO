#ifndef SILO_FLOAT_COLUMN_H
#define SILO_FLOAT_COLUMN_H

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
      archive& values;
      // clang-format on
   }

   std::vector<double> values;

  public:
   FloatColumnPartition();

   [[nodiscard]] const std::vector<double>& getValues() const;

   void insert(double value);
};

class FloatColumn {
   friend class boost::serialization::access;

  private:
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

#endif  // SILO_FLOAT_COLUMN_H
