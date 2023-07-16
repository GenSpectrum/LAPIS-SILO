#ifndef SILO_INSERTION_COLUMN_H
#define SILO_INSERTION_COLUMN_H

#include <cstdint>
#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "silo/common/bidirectional_map.h"
#include "silo/common/types.h"
#include "silo/storage/column/insertion_index.h"

namespace boost::serialization {
struct access;
}  // namespace boost::serialization

namespace silo::storage::column {

class InsertionColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive& values;
      // clang-format on
   }

   std::vector<silo::Idx> values;
   common::BidirectionalMap<std::string>& lookup;
   insertion::InsertionIndex insertion_index;

  public:
   explicit InsertionColumnPartition(common::BidirectionalMap<std::string>& lookup);

   void insert(const std::string& value);

   void buildInsertionIndex();

   [[nodiscard]] std::unique_ptr<roaring::Roaring> search(const std::string& search_pattern) const;

   [[nodiscard]] const std::vector<silo::Idx>& getValues() const;

   [[nodiscard]] std::string lookupValue(silo::Idx value_id) const;
};

class InsertionColumn {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      // clang-format on
   }

   std::deque<InsertionColumnPartition> partitions;
   std::unique_ptr<common::BidirectionalMap<std::string>> lookup;

  public:
   InsertionColumn();

   InsertionColumnPartition& createPartition();
};

}  // namespace silo::storage::column

#endif  // SILO_INSERTION_COLUMN_H
