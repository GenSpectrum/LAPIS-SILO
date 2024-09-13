#pragma once

#include <unordered_map>

#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/types.h"

namespace silo::storage {

class LineageIndex {
   friend class boost::serialization::access;

   common::LineageTree lineage_tree;
   std::unordered_map<Idx, roaring::Roaring> index;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & index;
      // clang-format on
   }

  public:
   LineageIndex(common::LineageTree lineage_tree);

   LineageIndex() = default;

   void insert(size_t row_id, Idx value);

   std::optional<const roaring::Roaring*> filterIncludingSublineages(Idx value_id) const;
};

}  // namespace silo::storage
