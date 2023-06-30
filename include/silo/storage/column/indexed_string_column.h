#ifndef SILO_INDEXED_STRING_COLUMN_H
#define SILO_INDEXED_STRING_COLUMN_H

#include <deque>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"

namespace boost::serialization {
struct access;
}

namespace silo::storage::column {

class IndexedStringColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      // clang-format off
      archive& value_ids;
      archive& indexed_values;
      // clang-format on
   }

   std::vector<Idx> value_ids;
   std::unordered_map<Idx, roaring::Roaring> indexed_values;
   common::BidirectionalMap<std::string>& lookup;

  public:
   explicit IndexedStringColumnPartition(common::BidirectionalMap<std::string>& lookup);

   [[nodiscard]] roaring::Roaring filter(const std::string& value) const;

   void insert(const std::string& value);

   const std::vector<silo::Idx>& getValues() const;

   inline std::string lookupValue(Idx id) const { return lookup.getValue(id); }
};

class IndexedStringColumn {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      // clang-format off
      archive& lookup;
      // clang-format on
      // TODO lookup sync
   }

   std::unique_ptr<common::BidirectionalMap<std::string>> lookup;
   std::deque<IndexedStringColumnPartition> partitions;

  public:
   IndexedStringColumn();

   IndexedStringColumnPartition& createPartition();
};

}  // namespace silo::storage::column

#endif  // SILO_INDEXED_STRING_COLUMN_H
