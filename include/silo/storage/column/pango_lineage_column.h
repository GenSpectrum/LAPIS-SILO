#ifndef SILO_PANGO_LINEAGE_COLUMN_H
#define SILO_PANGO_LINEAGE_COLUMN_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/bidirectional_map.h"
#include "silo/common/pango_lineage.h"
#include "silo/common/types.h"

namespace boost::serialization {
struct access;
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

   silo::common::BidirectionalMap<common::PangoLineage>& lookup;

   void insertSublineageValues(const common::PangoLineage& value, size_t row_number);

  public:
   explicit PangoLineageColumnPartition(common::BidirectionalMap<common::PangoLineage>& lookup);

   void insert(const common::PangoLineage& value);

   roaring::Roaring filter(const common::PangoLineage& value) const;

   roaring::Roaring filterIncludingSublineages(const common::PangoLineage& value) const;

   const std::vector<silo::Idx>& getValues() const;

   inline common::PangoLineage lookupValue(Idx id) const { return lookup.getValue(id); }
};

class PangoLineageColumn {
   friend class boost::serialization::access;

  private:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & *lookup;
      // clang-format on
   }

   std::unique_ptr<silo::common::BidirectionalMap<common::PangoLineage>> lookup;
   std::deque<PangoLineageColumnPartition> partitions;

  public:
   explicit PangoLineageColumn();

   PangoLineageColumnPartition& createPartition();
};

}  // namespace silo::storage::column

#endif  // SILO_PANGO_LINEAGE_COLUMN_H
