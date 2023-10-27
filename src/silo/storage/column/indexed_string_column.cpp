#include "silo/storage/column/indexed_string_column.h"

#include <optional>

#include "silo/common/bidirectional_map.h"
#include "silo/common/types.h"

namespace silo::storage::column {

IndexedStringColumnPartition::IndexedStringColumnPartition(
   common::BidirectionalMap<std::string>& lookup
)
    : lookup(lookup) {}

std::optional<const roaring::Roaring*> IndexedStringColumnPartition::filter(const std::string& value
) const {
   const auto value_id = lookup.getId(value);
   if (value_id.has_value() && indexed_values.contains(value_id.value())) {
      return &indexed_values.at(value_id.value());
   }
   return std::nullopt;
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
