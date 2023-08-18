#include "silo/storage/column/insertion_column.h"

#include <optional>

namespace silo::storage::column {

template <>
InsertionColumnPartition<NUCLEOTIDE_SYMBOL>::InsertionColumnPartition(
   common::BidirectionalMap<std::string>& lookup
)
    : lookup(lookup) {}

template <>
void InsertionColumnPartition<NUCLEOTIDE_SYMBOL>::insert(const std::string& value) {
   const auto sequence_id = values.size();

   const Idx value_id = lookup.getOrCreateId(value);
   values.push_back(value_id);

   insertion_index.addLazily(value, sequence_id);
}

template <>
void InsertionColumnPartition<NUCLEOTIDE_SYMBOL>::buildInsertionIndex() {
   insertion_index.buildIndex();
}

template <>
std::unique_ptr<roaring::Roaring> InsertionColumnPartition<NUCLEOTIDE_SYMBOL>::search(
   uint32_t position,
   const std::string& search_pattern
) const {
   return insertion_index.search(position, search_pattern);
}

template <>
const std::vector<silo::Idx>& InsertionColumnPartition<NUCLEOTIDE_SYMBOL>::getValues() const {
   return values;
}

template <>
std::string InsertionColumnPartition<NUCLEOTIDE_SYMBOL>::lookupValue(silo::Idx value_id) const {
   return lookup.getValue(value_id);
}

template <>
InsertionColumn<NUCLEOTIDE_SYMBOL>::InsertionColumn() {
   lookup = std::make_unique<silo::common::BidirectionalMap<std::string>>();
}

template <>
InsertionColumnPartition<NUCLEOTIDE_SYMBOL>& InsertionColumn<NUCLEOTIDE_SYMBOL>::createPartition() {
   return partitions.emplace_back(*lookup);
}

}  // namespace silo::storage::column
