#include "silo/storage/column/pango_lineage_column.h"

namespace silo::storage::column {

PangoLineageColumn::PangoLineageColumn() = default;

void PangoLineageColumn::insert(const common::PangoLineage& value) {
   if (!value_to_id_lookup.contains(value)) {
      for (const auto& parent_lineage : value.getParentLineages()) {
         initBitmapsForValue(parent_lineage);
      }
   }
   const uint32_t value_id = value_to_id_lookup[value];
   indexed_values[value_id].add(value_ids.size());
   insertSublineageValues(value);
   value_ids.push_back(value_id);
}

void PangoLineageColumn::initBitmapsForValue(const common::PangoLineage& value) {
   if (value_to_id_lookup.contains(value)) {
      return;
   }

   value_to_id_lookup[value] = value_to_id_lookup.size();
   id_to_value_lookup.push_back(value);
   indexed_values.emplace_back();
   indexed_sublineage_values.emplace_back();
}

void PangoLineageColumn::insertSublineageValues(const common::PangoLineage& value) {
   for (const auto& pango_lineage : value.getParentLineages()) {
      const uint32_t value_id = value_to_id_lookup[pango_lineage];
      indexed_sublineage_values[value_id].add(value_ids.size());
   }
}

roaring::Roaring PangoLineageColumn::filter(const common::PangoLineage& value) const {
   if (!value_to_id_lookup.contains(value)) {
      return {};
   }

   return indexed_values[value_to_id_lookup.at(value)];
}

roaring::Roaring PangoLineageColumn::filterIncludingSublineages(const common::PangoLineage& value
) const {
   if (!value_to_id_lookup.contains(value)) {
      return {};
   }

   return indexed_sublineage_values[value_to_id_lookup.at(value)];
}

std::string PangoLineageColumn::getAsString(std::size_t idx) const {
   return id_to_value_lookup.at(value_ids.at(idx)).value;
};

}  // namespace silo::storage::column
