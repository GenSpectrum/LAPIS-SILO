#pragma once

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"
#include "silo/common/types.h"

namespace boost::serialization {
struct access;
}

namespace silo::storage::column {

class IndexedStringColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & value_ids;
      archive & indexed_values;
      // clang-format on
   }

   std::vector<Idx> value_ids;
   std::unordered_map<Idx, roaring::Roaring> indexed_values;
   common::BidirectionalMap<std::string>& lookup;

  public:
   explicit IndexedStringColumnPartition(common::BidirectionalMap<std::string>& lookup);

   [[nodiscard]] std::optional<const roaring::Roaring*> filter(const std::string& value) const;

   void insert(const std::string& value);

   void insertNull();

   [[nodiscard]] const std::vector<silo::Idx>& getValues() const;

   [[nodiscard]] inline std::string lookupValue(Idx id) const { return lookup.getValue(id); }
};

class IndexedStringColumn {
   friend class boost::serialization::access;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & *lookup;
      // clang-format on
   }

   std::unique_ptr<common::BidirectionalMap<std::string>> lookup;
   std::deque<IndexedStringColumnPartition> partitions;

  public:
   IndexedStringColumn();

   IndexedStringColumnPartition& createPartition();
};

}  // namespace silo::storage::column
