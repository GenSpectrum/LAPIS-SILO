#include "silo/storage/column/string_column.h"

#include <string>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/german_string.h"
#include "silo/common/tree_node_id.h"
#include "silo/initialize/initialize_exception.h"

using silo::common::TreeNodeId;

namespace silo::storage::column {

StringColumn::StringColumn(StringColumnMetadata* metadata)
    : metadata(metadata) {}

namespace {
std::expected<void, std::string> insertValue(
   std::string_view value,
   StringColumnMetadata* metadata,
   vector::GermanStringRegistry& fixed_string_data,
   vector::VariableDataRegistry& variable_string_data
) {
   size_t row_id;
   if (value.size() <= SiloString::SHORT_STRING_SIZE) {
      row_id = fixed_string_data.insert(SiloString{value});
   } else {
      SILO_ASSERT(value.length() < UINT32_MAX);
      auto suffix_id = variable_string_data.insert(value.substr(SiloString::PREFIX_LENGTH));
      row_id = fixed_string_data.insert(SiloString{
         static_cast<uint32_t>(value.length()),
         value.substr(0, SiloString::PREFIX_LENGTH),
         suffix_id
      });
   }
   if (metadata->phylo_tree.has_value()) {
      auto child_it = (metadata->phylo_tree->nodes).find(TreeNodeId{std::string{value}});
      if (child_it == metadata->phylo_tree->nodes.end()) {
         return {};
      }
      if (child_it->second->rowIndexExists()) {
         return std::unexpected(
            fmt::format("Node '{}' already exists in the phylogenetic tree.", value)
         );
      }
      child_it->second->row_index = row_id;
   }
   return {};
}
}  // namespace

std::expected<void, std::string> StringColumn::appendChunk(const Buffer& buffer) {
   for (const auto& value : buffer) {
      if (value.has_value()) {
         auto result = insertValue(*value, metadata, fixed_string_data, variable_string_data);
         if (!result.has_value()) {
            return result;
         }
      } else {
         null_bitmap.add(fixed_string_data.numValues());
         fixed_string_data.insert(SiloString(""));
      }
   }
   return {};
}

bool StringColumn::isNull(size_t row_id) const {
   return null_bitmap.contains(row_id);
}

}  // namespace silo::storage::column
