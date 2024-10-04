#pragma once

#include <filesystem>
#include <unordered_map>

#include <boost/serialization/access.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/types.h"

namespace silo::storage {

// Forward declaration for the friend class access of IndexedStringColumn
// which is the only allowed user of this class (/this class's constructor).
// This ensures safety of the lineage_tree pointer, which
// (i) is guaranteed to be initialized (and const)  by the constructor and stays valid as
// (ii) the lifetime of this index is bound to the containing column_partition containing it
namespace column {
class IndexedStringColumnPartition;
}

class LineageIndex {
   friend class column::IndexedStringColumnPartition;
   friend class boost::serialization::access;

   const common::LineageTree* lineage_tree;
   std::unordered_map<Idx, roaring::Roaring> index_including_sublineages;
   std::unordered_map<Idx, roaring::Roaring> index_excluding_sublineages;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & index_including_sublineages;
      archive & index_excluding_sublineages;
      // clang-format on
   }

   explicit LineageIndex(const common::LineageTree* lineage_tree);

  public:
   void insert(size_t row_id, Idx value_id);

   std::optional<const roaring::Roaring*> filterIncludingSublineages(Idx value_id) const;

   std::optional<const roaring::Roaring*> filterExcludingSublineages(Idx value_id) const;
};

}  // namespace silo::storage
