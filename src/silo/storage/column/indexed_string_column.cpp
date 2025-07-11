#include "silo/storage/column/indexed_string_column.h"

#include <optional>

#include <fmt/format.h>

#include "silo/common/bidirectional_string_map.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::storage::column {

IndexedStringColumnMetadata::IndexedStringColumnMetadata(
   std::string column_name,
   common::LineageTreeAndIdMap lineage_tree_and_id_map
)
    : ColumnMetadata(std::move(column_name)),
      dictionary(lineage_tree_and_id_map.lineage_id_lookup_map.copy()),
      lineage_tree(std::move(lineage_tree_and_id_map)) {}

IndexedStringColumnMetadata::IndexedStringColumnMetadata(
   std::string column_name,
   common::BidirectionalStringMap dictionary,
   common::LineageTreeAndIdMap lineage_tree_and_id_map
)
    : ColumnMetadata(std::move(column_name)),
      dictionary(std::move(dictionary)),
      lineage_tree(std::move(lineage_tree_and_id_map)) {}

IndexedStringColumnPartition::IndexedStringColumnPartition(IndexedStringColumnMetadata* metadata)
    : metadata(metadata) {
   if (metadata->lineage_tree.has_value()) {
      lineage_index = LineageIndex{&metadata->lineage_tree->lineage_tree};
   }
}

std::optional<const roaring::Roaring*> IndexedStringColumnPartition::filter(Idx value_id) const {
   if (indexed_values.contains(value_id)) {
      return &indexed_values.at(value_id);
   }
   return std::nullopt;
}

std::optional<const roaring::Roaring*> IndexedStringColumnPartition::filter(
   const std::optional<std::string>& value
) const {
   const auto& value_id = metadata->dictionary.getId(value.value_or(""));
   if (!value_id.has_value()) {
      return std::nullopt;
   }
   return filter(value_id.value());
}

void IndexedStringColumnPartition::insert(const std::string& value) {
   const size_t row_id = value_ids.size();

   if (lineage_index.has_value()) {
      const auto value_id = metadata->dictionary.getId(value);
      if (!value_id.has_value()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The value '{}' is not a valid lineage value for column '{}'. "
            "Is your lineage definition file outdated?",
            value,
            metadata->column_name
         ));
      }
      lineage_index->insert(row_id, value_id.value());
   }

   const Idx value_id = metadata->dictionary.getOrCreateId(value);

   indexed_values[value_id].add(row_id);
   value_ids.push_back(value_id);
}

void IndexedStringColumnPartition::insertNull() {
   const Idx value_id = metadata->dictionary.getOrCreateId("");

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
   return metadata->dictionary.getId(value);
}

const std::optional<LineageIndex>& IndexedStringColumnPartition::getLineageIndex() const {
   return lineage_index;
}

}  // namespace silo::storage::column
