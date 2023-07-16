#include "silo/storage/column/insertion_column.h"

#include <optional>

namespace silo::storage::column {

InsertionColumnPartition::InsertionColumnPartition(common::BidirectionalMap<std::string>& lookup)
    : lookup(lookup) {}

void InsertionColumnPartition::insert(const std::string& value) {
   const auto sequence_id = values.size();

   const Idx value_id = lookup.getOrCreateId(value);
   values.push_back(value_id);

   insertion_index.addLazily(value, sequence_id);
}

void InsertionColumnPartition::buildInsertionIndex() {
   insertion_index.buildIndex();
}

std::unique_ptr<roaring::Roaring> InsertionColumnPartition::search(const std::string& search_pattern
) const {
   return insertion_index.search(search_pattern);
}

const std::vector<silo::Idx>& InsertionColumnPartition::getValues() const {
   return values;
}

std::string InsertionColumnPartition::lookupValue(silo::Idx value_id) const {
   return lookup.getValue(value_id);
}

InsertionColumn::InsertionColumn() {
   lookup = std::make_unique<silo::common::BidirectionalMap<std::string>>();
}

InsertionColumnPartition& InsertionColumn::createPartition() {
   return partitions.emplace_back(*lookup);
}

}  // namespace silo::storage::column
