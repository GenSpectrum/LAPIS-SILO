#include "silo/storage/column/indexed_string_column.h"

#include <optional>

#include <fmt/format.h>

#include "silo/common/bidirectional_map.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column {

IndexedStringColumnPartition::IndexedStringColumnPartition(
   common::BidirectionalMap<std::string>& lookup
)
    : lookup(lookup) {}

std::optional<const roaring::Roaring*> IndexedStringColumnPartition::filter(Idx value_id) const {
   if (indexed_values.contains(value_id)) {
      return &indexed_values.at(value_id);
   }
   return std::nullopt;
}

std::optional<const roaring::Roaring*> IndexedStringColumnPartition::filter(
   const std::optional<std::string>& value
) const {
   const auto& value_id = lookup.getId(value.value_or(""));
   if (!value_id.has_value()) {
      return std::nullopt;
   }
   return filter(value_id.value());
}

void IndexedStringColumnPartition::insert(const std::string& value) {
   const size_t row_id = value_ids.size();

   if (lineage_index.has_value()) {
      const auto value_id = lookup.getId(value);
      if (!value_id.has_value()) {
         throw silo::preprocessing::PreprocessingException(
            fmt::format("The value '{}' is not a valid lineage value.", value)
         );
      }
      lineage_index->insert(row_id, value_id.value());
   }

   const Idx value_id = lookup.getOrCreateId(value);

   indexed_values[value_id].add(row_id);
   value_ids.push_back(value_id);
}

void IndexedStringColumnPartition::insertNull() {
   const Idx value_id = lookup.getOrCreateId("");

   indexed_values[value_id].add(value_ids.size());
   value_ids.push_back(value_id);
}

void IndexedStringColumnPartition::reserve(size_t row_count) {
   value_ids.reserve(value_ids.size() + row_count);
}

const std::vector<silo::Idx>& IndexedStringColumnPartition::getValues() const {
   return this->value_ids;
}

std::optional<silo::Idx> IndexedStringColumnPartition::getValueId(const std::string& value) const {
   return lookup.getId(value);
}

const std::optional<LineageIndex>& IndexedStringColumnPartition::getLineageIndex() const {
   return lineage_index;
}

IndexedStringColumn::IndexedStringColumn() {
   lookup = std::make_unique<common::BidirectionalMap<std::string>>();
}

IndexedStringColumnPartition& IndexedStringColumn::createPartition() {
   return partitions.emplace_back(*lookup);
}

void IndexedStringColumn::generateLineageIndex(const common::LineageTreeAndIDMap& lineage_tree) {
   *lookup = lineage_tree.lineage_id_lookup_map.copy();
   for (auto& partition : partitions) {
      std::reference_wrapper(partition.lookup) = *lookup;
      partition.lineage_index = LineageIndex(lineage_tree.lineage_tree);
   }
}

}  // namespace silo::storage::column
