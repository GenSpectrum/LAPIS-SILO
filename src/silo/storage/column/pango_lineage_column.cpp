#include "silo/storage/column/pango_lineage_column.h"

namespace silo::storage::column {

PangoLineageColumn::PangoLineageColumn() = default;

void PangoLineageColumn::insert(const common::PangoLineage& value) {
   if (!value_id_lookup.contains(value)) {
      for (const auto& parent_lineage : value.getParentLineages()) {
         initBitmapsForValue(parent_lineage);
      }
   }

   const auto value_id = value_id_lookup[value];
   indexed_values[value_id].add(sequence_count);
   insertSublineageValues(value);
   sequence_count++;
}

void PangoLineageColumn::initBitmapsForValue(const common::PangoLineage& value) {
   if (value_id_lookup.contains(value)) {
      return;
   }

   value_id_lookup[value] = value_id_lookup.size();
   indexed_values.emplace_back();

   roaring::Roaring sublineage_values;
   for (const auto& [pango_lineage, value_id] : value_id_lookup) {
      if (pango_lineage.isSublineageOf(value)) {
         sublineage_values |= indexed_values[value_id];
      }
   }

   indexed_sublineage_values.emplace_back(sublineage_values);
}

void PangoLineageColumn::insertSublineageValues(const common::PangoLineage& value) {
   for (const auto& [pango_lineage, value_id] : value_id_lookup) {
      if (value.isSublineageOf(pango_lineage)) {
         indexed_sublineage_values[value_id].add(sequence_count);
      }
   }
}

roaring::Roaring PangoLineageColumn::filter(const common::PangoLineage& value) const {
   if (!value_id_lookup.contains(value)) {
      return {};
   }

   return indexed_values[value_id_lookup.at(value)];
}

roaring::Roaring PangoLineageColumn::filterIncludingSublineages(const common::PangoLineage& value
) const {
   if (!value_id_lookup.contains(value)) {
      return {};
   }

   return indexed_sublineage_values[value_id_lookup.at(value)];
}

}  // namespace silo::storage::column
