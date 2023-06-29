#include "silo/storage/column/indexed_string_column.h"

namespace silo::storage::column {

IndexedStringColumnPartition::IndexedStringColumnPartition(
   common::BidirectionalMap<std::string>& lookup
)
    : lookup(lookup) {}

roaring::Roaring IndexedStringColumnPartition::filter(const std::string& value) const {
   auto value_id = lookup.getId(value);
   if (value_id.has_value()) {
      return indexed_values.at(value_id.value());
   }
   return {};
}

void IndexedStringColumnPartition::insert(const std::string& value) {
   const Idx value_id = lookup.getOrCreateId(value);

   indexed_values[value_id].add(value_ids.size());
   value_ids.push_back(value_id);
}

const std::vector<silo::Idx>& IndexedStringColumnPartition::getValues() const {
   return this->value_ids;
}

IndexedStringColumn::IndexedStringColumn() {
   lookup = std::make_unique<common::BidirectionalMap<std::string>>();
}

IndexedStringColumnPartition& IndexedStringColumn::createPartition() {
   return partitions.emplace_back(*lookup);
}

}  // namespace silo::storage::column
