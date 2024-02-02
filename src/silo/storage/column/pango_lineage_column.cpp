#include "silo/storage/column/pango_lineage_column.h"

#include <optional>
#include <utility>

#include "silo/common/bidirectional_map.h"
#include "silo/common/pango_lineage.h"
#include "silo/storage/pango_lineage_alias.h"

namespace silo::storage::column {

PangoLineageColumnPartition::PangoLineageColumnPartition(
   silo::PangoLineageAliasLookup& alias_key,
   common::BidirectionalMap<common::UnaliasedPangoLineage>& lookup_unaliased,
   common::BidirectionalMap<common::AliasedPangoLineage>& lookup_aliased
)
    : alias_key(alias_key),
      lookup_unaliased(lookup_unaliased),
      lookup_aliased(lookup_aliased) {}

void PangoLineageColumnPartition::insert(const common::RawPangoLineage& value) {
   const common::UnaliasedPangoLineage resolved_lineage = alias_key.unaliasPangoLineage(value);
   for (const auto& parent_lineage : resolved_lineage.getParentLineages()) {
      (void)lookup_unaliased.getOrCreateId(parent_lineage);
      (void)lookup_aliased.getOrCreateId(alias_key.aliasPangoLineage(parent_lineage));
   }
   const Idx value_id = lookup_unaliased.getOrCreateId(resolved_lineage);
   (void)lookup_aliased.getOrCreateId(alias_key.aliasPangoLineage(resolved_lineage));

   const size_t row_number = value_ids.size();
   value_ids.push_back(value_id);
   indexed_values[value_id].add(row_number);
   insertSublineageValues(resolved_lineage, row_number);
}

void PangoLineageColumnPartition::insertNull() {
   insert({""});
}

void PangoLineageColumnPartition::reserve(size_t row_count) {
   value_ids.reserve(value_ids.size() + row_count);
}

void PangoLineageColumnPartition::insertSublineageValues(
   const common::UnaliasedPangoLineage& value,
   size_t row_number
) {
   for (const auto& pango_lineage : value.getParentLineages()) {
      const Idx value_id = lookup_unaliased.getOrCreateId(pango_lineage);
      indexed_sublineage_values[value_id].add(row_number);

      const auto aliased_lineage = alias_key.aliasPangoLineage(pango_lineage);
      (void)lookup_aliased.getOrCreateId(aliased_lineage);
   }
}

std::optional<const roaring::Roaring*> PangoLineageColumnPartition::filter(
   const common::RawPangoLineage& value
) const {
   const common::UnaliasedPangoLineage resolved_lineage = alias_key.unaliasPangoLineage(value);
   auto value_id = lookup_unaliased.getId(resolved_lineage);
   if (value_id.has_value() && indexed_values.contains(value_id.value())) {
      return &indexed_values.at(value_id.value());
   }
   return std::nullopt;
}

std::optional<const roaring::Roaring*> PangoLineageColumnPartition::filterIncludingSublineages(
   const common::RawPangoLineage& value
) const {
   const common::UnaliasedPangoLineage resolved_lineage = alias_key.unaliasPangoLineage(value);
   auto value_id = lookup_unaliased.getId(resolved_lineage);
   if (value_id.has_value() && indexed_sublineage_values.contains(value_id.value())) {
      return &indexed_sublineage_values.at(value_id.value());
   }
   return std::nullopt;
}

const std::vector<silo::Idx>& PangoLineageColumnPartition::getValues() const {
   return this->value_ids;
}
common::AliasedPangoLineage PangoLineageColumnPartition::lookupAliasedValue(Idx idx) const {
   return lookup_aliased.getValue(idx);
}

common::UnaliasedPangoLineage PangoLineageColumnPartition::lookupUnaliasedValue(Idx idx) const {
   return lookup_unaliased.getValue(idx);
}

PangoLineageColumn::PangoLineageColumn(silo::PangoLineageAliasLookup alias_key) {
   lookup_unaliased = std::make_unique<common::BidirectionalMap<common::UnaliasedPangoLineage>>();
   lookup_aliased = std::make_unique<common::BidirectionalMap<common::AliasedPangoLineage>>();
   this->alias_key = std::make_unique<PangoLineageAliasLookup>(std::move(alias_key));
}

PangoLineageColumnPartition& PangoLineageColumn::createPartition() {
   return partitions.emplace_back(*alias_key, *lookup_unaliased, *lookup_aliased);
}

}  // namespace silo::storage::column
