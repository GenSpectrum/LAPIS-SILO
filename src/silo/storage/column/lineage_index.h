#pragma once

#include <filesystem>
#include <unordered_map>

#include <boost/serialization/access.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <roaring/roaring.hh>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/types.h"

namespace silo::storage {

class LineageIndex {
   friend class boost::serialization::access;

   const common::LineageTree* lineage_tree;
   std::unordered_map<Idx, roaring::Roaring> index_excluding_sublineages;
   std::unordered_map<
      silo::common::RecombinantEdgeFollowingMode,
      std::unordered_map<Idx, roaring::Roaring>>
      index_including_sublineages;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & index_including_sublineages;
      archive & index_excluding_sublineages;
      // clang-format on
   }

  public:
   explicit LineageIndex(const common::LineageTree* lineage_tree);

   void insert(size_t row_id, Idx value_id);

   std::optional<const roaring::Roaring*> filterIncludingSublineages(
      Idx value_id,
      silo::common::RecombinantEdgeFollowingMode recombinant_edge_following_mode
   ) const;

   std::optional<const roaring::Roaring*> filterExcludingSublineages(Idx value_id) const;
};

}  // namespace silo::storage
