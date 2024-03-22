#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"
#include "silo/common/pango_lineage.h"
#include "silo/common/types.h"
#include "silo/storage/pango_lineage_alias.h"

namespace boost::serialization {
class access;
}

namespace silo::storage::column {

class PangoLineageColumnPartition {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & value_ids;
      archive & indexed_values;
      archive & indexed_sublineage_values;
      // clang-format on
   }

   std::vector<Idx> value_ids;
   std::unordered_map<Idx, roaring::Roaring> indexed_values;
   std::unordered_map<Idx, roaring::Roaring> indexed_sublineage_values;

   silo::PangoLineageAliasLookup& alias_key;
   silo::common::BidirectionalMap<common::UnaliasedPangoLineage>& lookup_unaliased;
   silo::common::BidirectionalMap<common::AliasedPangoLineage>& lookup_aliased;

   void insertSublineageValues(const common::UnaliasedPangoLineage& value, size_t row_number);

  public:
   explicit PangoLineageColumnPartition(
      silo::PangoLineageAliasLookup& alias_key,
      common::BidirectionalMap<common::UnaliasedPangoLineage>& lookup_unaliased,
      common::BidirectionalMap<common::AliasedPangoLineage>& lookup_aliased
   );

   void insert(const common::RawPangoLineage& value);

   void insertNull();

   void reserve(size_t row_count);

   std::optional<const roaring::Roaring*> filter(const common::RawPangoLineage& value) const;

   std::optional<const roaring::Roaring*> filterIncludingSublineages(
      const common::RawPangoLineage& value
   ) const;

   const std::vector<silo::Idx>& getValues() const;

   common::AliasedPangoLineage lookupAliasedValue(Idx idx) const;
   common::UnaliasedPangoLineage lookupUnaliasedValue(Idx idx) const;
};

class PangoLineageColumn {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & *lookup_unaliased;
      archive & *lookup_aliased;
      // clang-format on
   }

   std::unique_ptr<silo::common::BidirectionalMap<common::UnaliasedPangoLineage>> lookup_unaliased;
   std::unique_ptr<silo::common::BidirectionalMap<common::AliasedPangoLineage>> lookup_aliased;
   std::unique_ptr<silo::PangoLineageAliasLookup> alias_key;
   std::deque<PangoLineageColumnPartition> partitions;

  public:
   explicit PangoLineageColumn(silo::PangoLineageAliasLookup alias_key);

   PangoLineageColumnPartition& createPartition();
};

}  // namespace silo::storage::column
