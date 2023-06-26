#include "silo/storage/column/pango_lineage_column.h"

#include "silo/common/bidirectional_map.h"

namespace silo::storage::column {

PangoLineageColumnPartition::PangoLineageColumnPartition(
   common::BidirectionalMap<common::PangoLineage>& lookup
)
    : lookup(lookup){};

void PangoLineageColumnPartition::insert(const common::PangoLineage& value) {
   for (const auto& parent_lineage : value.getParentLineages()) {
      (void)lookup.getOrCreateId(parent_lineage);
   }
   const Idx value_id = lookup.getOrCreateId(value);
   const TID row_number = value_ids.size();
   indexed_values[value_id].add(row_number);
   insertSublineageValues(value, row_number);
   value_ids.push_back(value_id);
}

void PangoLineageColumnPartition::insertSublineageValues(
   const common::PangoLineage& value,
   TID row_number
) {
   for (const auto& pango_lineage : value.getParentLineages()) {
      Idx value_id = lookup.getOrCreateId(pango_lineage);
      indexed_sublineage_values[value_id].add(row_number);
   }
}

roaring::Roaring PangoLineageColumnPartition::filter(const common::PangoLineage& value) const {
   auto value_id = lookup.getId(value);
   if (value_id.has_value() && indexed_values.contains(value_id.value())) {
      return indexed_values.at(value_id.value());
   }
   return {};
}

roaring::Roaring PangoLineageColumnPartition::filterIncludingSublineages(
   const common::PangoLineage& value
) const {
   auto value_id = lookup.getId(value);
   if (value_id.has_value() && indexed_sublineage_values.contains(value_id.value())) {
      return indexed_sublineage_values.at(value_id.value());
   }
   return {};
}

const std::vector<silo::Idx>& PangoLineageColumnPartition::getValues() const {
   return this->value_ids;
}

PangoLineageColumn::PangoLineageColumn() {
   lookup = std::make_unique<common::BidirectionalMap<common::PangoLineage>>();
};

PangoLineageColumnPartition PangoLineageColumn::createPartition() {
   return PangoLineageColumnPartition(*lookup);
}

}  // namespace silo::storage::column
