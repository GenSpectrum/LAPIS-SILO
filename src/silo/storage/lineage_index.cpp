#include "silo/storage/lineage_index.h"

#include <spdlog/spdlog.h>

namespace silo::storage {

LineageIndex::LineageIndex(const common::LineageTree* lineage_tree)
    : lineage_tree(lineage_tree) {}

void LineageIndex::insert(size_t row_id, Idx value_id) {
   value_id = lineage_tree->resolveAlias(value_id);
   index_excluding_sublineages[value_id].add(row_id);
   index_including_sublineages[value_id].add(row_id);
   std::optional<Idx> current = value_id;
   while ((current = lineage_tree->getParent(current.value()))) {
      index_including_sublineages[current.value()].add(row_id);
   }
}

std::optional<const roaring::Roaring*> LineageIndex::filterIncludingSublineages(Idx value_id
) const {
   value_id = lineage_tree->resolveAlias(value_id);
   if (index_including_sublineages.contains(value_id)) {
      return &index_including_sublineages.at(value_id);
   }
   return std::nullopt;
}

std::optional<const roaring::Roaring*> LineageIndex::filterExcludingSublineages(Idx value_id
) const {
   value_id = lineage_tree->resolveAlias(value_id);
   if (index_excluding_sublineages.contains(value_id)) {
      return &index_excluding_sublineages.at(value_id);
   }
   return std::nullopt;
}

}  // namespace silo::storage
