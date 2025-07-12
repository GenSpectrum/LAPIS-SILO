#include "silo/storage/column/string_column.h"

#include <string>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/string.h"
#include "silo/common/tree_node_id.h"
#include "silo/initialize/initialize_exception.h"

using silo::common::String;
using silo::common::STRING_SIZE;
using silo::common::TreeNodeId;

namespace silo::storage::column {

std::optional<String<STRING_SIZE>> StringColumnMetadata::embedString(const std::string& string
) const {
   return String<STRING_SIZE>::embedString(string, dictionary);
}

StringColumnPartition::StringColumnPartition(StringColumnMetadata* metadata)
    : metadata(metadata) {}

void StringColumnPartition::insert(std::string_view value) {
   const String<STRING_SIZE> tmp(value, metadata->dictionary);
   values.push_back(tmp);
   if (metadata->phylo_tree.has_value()) {
      auto child_it = (metadata->phylo_tree->nodes).find(TreeNodeId{std::string{value}});
      if (child_it == metadata->phylo_tree->nodes.end()) {
         return;
      }
      if (child_it->second->rowIndexExists()) {
         throw silo::initialize::InitializeException(
            fmt::format("Node '{}' already exists in the phylogenetic tree.", value)
         );
      }
      child_it->second->row_index = values.size() - 1;
   }
}

void StringColumnPartition::insertNull() {
   const String<STRING_SIZE> tmp("", metadata->dictionary);
   values.push_back(tmp);
}

void StringColumnPartition::reserve(size_t row_count) {
   values.reserve(values.size() + row_count);
}

const std::vector<String<STRING_SIZE>>& StringColumnPartition::getValues() const {
   return values;
}

std::optional<String<STRING_SIZE>> StringColumnPartition::embedString(const std::string& string
) const {
   return String<STRING_SIZE>::embedString(string, metadata->dictionary);
}

}  // namespace silo::storage::column
