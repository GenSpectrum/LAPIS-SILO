#ifndef SILO_PANGO_LINEAGE_COLUMN_H
#define SILO_PANGO_LINEAGE_COLUMN_H

#include <unordered_map>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/pango_lineage.h"

namespace silo::storage::column {

class PangoLineageColumn {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& value_id_lookup;
      archive& indexed_values;
   }

  private:
   std::unordered_map<common::PangoLineage, uint32_t> value_id_lookup;
   std::vector<roaring::Roaring> indexed_values;
   std::vector<roaring::Roaring> indexed_sublineage_values;
   uint32_t sequence_count = 0;

   void insertSublineageValues(const common::PangoLineage& value);

  public:
   PangoLineageColumn();

   void insert(const common::PangoLineage& value);

   roaring::Roaring filter(const common::PangoLineage& value) const;

   roaring::Roaring filterIncludingSublineages(const common::PangoLineage& value) const;
};

}  // namespace silo::storage::column

#endif  // SILO_PANGO_LINEAGE_COLUMN_H
