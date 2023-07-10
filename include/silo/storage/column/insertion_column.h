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

namespace boost::serialization {
struct access;
}

namespace silo::storage::column {

class InsertionColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      // TODO(#164) serialize data-structures
      // clang-format on
   }

   // TODO(#164) remove this and add special datastructures here
   std::vector<silo::Idx> values;
   common::BidirectionalMap<std::string>& lookup;

  public:
   explicit InsertionColumnPartition(
      common::BidirectionalMap<std::string>& lookup
      // TODO(#164) add datastructures that need to be synchronized across partitions here
   );

   void insert(const std::string& value);

   // TODO(#164) Return a type that is byte-wise comparable
   [[nodiscard]] const std::vector<silo::Idx>& getValues() const;

   // TODO(#164) Maybe require helper function to return the original string value from the
   //  internal representation type used for querying
   [[nodiscard]] std::string lookupValue(silo::Idx value_id) const;
};

class InsertionColumn {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      // TODO(#164) serialize data-structures
      // clang-format on
   }

   std::deque<InsertionColumnPartition> partitions;
   std::unique_ptr<common::BidirectionalMap<std::string>> lookup;

   // TODO(#164) synchronized data-structures

  public:
   InsertionColumn();

   InsertionColumnPartition& createPartition();
};

}  // namespace silo::storage::column

#endif  // SILO_INSERTION_COLUMN_H
