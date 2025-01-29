#include "silo/storage/column/indexed_string_column.h"

#include <optional>

#include <fmt/format.h>

#include "silo/common/bidirectional_map.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column {

IndexedStringColumnPartition::IndexedStringColumnPartition(
   std::string column_name,
   common::BidirectionalMap<std::string>* lookup
)
    : column_name(std::move(column_name)),
      lookup(lookup) {}

IndexedStringColumnPartition::IndexedStringColumnPartition(
   std::string column_name,
   common::BidirectionalMap<std::string>* lookup,
   const common::LineageTree* lineage_tree
)
    : column_name(std::move(column_name)),
      lineage_index(LineageIndex(lineage_tree)),
      lookup(lookup) {}

std::optional<const roaring::Roaring*> IndexedStringColumnPartition::filter(Idx value_id) const {
   if (indexed_values.contains(value_id)) {
      return &indexed_values.at(value_id);
   }
   return std::nullopt;
}

std::optional<const roaring::Roaring*> IndexedStringColumnPartition::filter(
   const std::optional<std::string>& value
) const {
   const auto& value_id = lookup->getId(value.value_or(""));
   if (!value_id.has_value()) {
      return std::nullopt;
   }
   return filter(value_id.value());
}

void IndexedStringColumnPartition::insert(const std::string& value) {
   const size_t row_id = value_ids.size();

   if (lineage_index.has_value()) {
      const auto value_id = lookup->getId(value);
      if (!value_id.has_value()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The value '{}' is not a valid lineage value for column '{}'. "
            "Is your lineage definition file outdated?",
            value,
            column_name
         ));
      }
      lineage_index->insert(row_id, value_id.value());
   }

   const Idx value_id = lookup->getOrCreateId(value);

   indexed_values[value_id].add(row_id);
   value_ids.push_back(value_id);
}

void IndexedStringColumnPartition::insertNull() {
   const Idx value_id = lookup->getOrCreateId("");

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
   return lookup->getId(value);
}

const std::optional<LineageIndex>& IndexedStringColumnPartition::getLineageIndex() const {
   return lineage_index;
}

IndexedStringColumn::IndexedStringColumn(std::string column_name)
    : column_name(std::move(column_name)) {}

IndexedStringColumn::IndexedStringColumn(
   std::string column_name,
   const common::LineageTreeAndIdMap& lineage_tree_and_id_map
)
    : column_name(std::move(column_name)) {
   lineage_tree = lineage_tree_and_id_map.lineage_tree;
   lookup = lineage_tree_and_id_map.lineage_id_lookup_map.copy();
}

IndexedStringColumnPartition& IndexedStringColumn::createPartition() {
   if (lineage_tree.has_value()) {
      return partitions.emplace_back(
         IndexedStringColumnPartition{column_name, &lookup, &lineage_tree.value()}
      );
   }
   return partitions.emplace_back(IndexedStringColumnPartition{column_name, &lookup});
}

bool IndexedStringColumn::hasLineageTree() const {
   return lineage_tree.has_value();
}

}  // namespace silo::storage::column
