#ifndef SILO_PANGO_LINEAGE_COLUMN_H
#define SILO_PANGO_LINEAGE_COLUMN_H

#include <unordered_map>
#include <vector>

#include <roaring/roaring.hh>

#include "silo/common/pango_lineage.h"
#include "silo/storage/column/column.h"

namespace silo::storage::column {

class PangoLineageColumn : public Column {
  public:
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const unsigned int /* version */) {
      archive& value_ids;
      archive& value_to_id_lookup;
      archive& id_to_value_lookup;
      archive& indexed_values;
      archive& indexed_sublineage_values;
   }

  private:
   std::vector<uint32_t> value_ids;
   std::unordered_map<common::PangoLineage, uint32_t> value_to_id_lookup;
   std::vector<common::PangoLineage> id_to_value_lookup;
   std::vector<roaring::Roaring> indexed_values;
   std::vector<roaring::Roaring> indexed_sublineage_values;

   void insertSublineageValues(const common::PangoLineage& value);

   void initBitmapsForValue(const common::PangoLineage& value);

  public:
   PangoLineageColumn();

   void insert(const common::PangoLineage& value);

   roaring::Roaring filter(const common::PangoLineage& value) const;

   roaring::Roaring filterIncludingSublineages(const common::PangoLineage& value) const;

   std::string getAsString(std::size_t idx) const override;
};

}  // namespace silo::storage::column

#endif  // SILO_PANGO_LINEAGE_COLUMN_H
