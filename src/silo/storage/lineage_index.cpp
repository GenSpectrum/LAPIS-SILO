#include "silo/storage/lineage_index.h"

namespace silo::storage {

LineageIndex::LineageIndex(common::LineageTree lineage_tree)
    : lineage_tree(std::move(lineage_tree)) {}

void LineageIndex::insert(size_t row_id, Idx value) {
   index[value].add(row_id);
   std::optional<Idx> current = value;
   while ((current = lineage_tree.getParent(current.value()))) {
      index[current.value()].add(row_id);
   }
}

std::optional<const roaring::Roaring*> LineageIndex::filterIncludingSublineages(Idx value_id
) const {
   if (index.contains(value_id)) {
      return &index.at(value_id);
   }
   return std::nullopt;
}

}  // namespace silo::storage
