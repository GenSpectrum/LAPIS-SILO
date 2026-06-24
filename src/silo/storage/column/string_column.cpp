#include "silo/storage/column/string_column.h"

#include <algorithm>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "silo/common/bidirectional_string_map.h"
#include "silo/common/phylo_tree.h"
#include "silo/common/tree_node_id.h"
#include "silo/initialize/initialize_exception.h"

using silo::common::TreeNodeId;

namespace silo::storage::column {

size_t StringColumnChunk::insert(std::string_view value) {
   if (value.size() <= SiloString::SHORT_STRING_SIZE) {
      return fixed_string_data.insert(SiloString{value});
   }
   SILO_ASSERT(value.length() < UINT32_MAX);
   auto suffix_id = variable_string_data.insert(value.substr(SiloString::PREFIX_LENGTH));
   return fixed_string_data.insert(SiloString{
      static_cast<uint32_t>(value.length()), value.substr(0, SiloString::PREFIX_LENGTH), suffix_id
   });
}

size_t StringColumnChunk::insertNull() {
   return fixed_string_data.insert(SiloString(""));
}

size_t StringColumnChunk::numValues() const {
   return fixed_string_data.numValues();
}

SiloString StringColumnChunk::getValue(size_t row_in_chunk) const {
   return fixed_string_data.get(row_in_chunk);
}

std::string StringColumnChunk::lookupValue(SiloString string) const {
   if (string.isInPlace()) {
      auto string_view = string.getShortString();
      return std::string{string_view};
   }
   std::string result;
   result.reserve(string.length());
   result += string.prefix();

   auto suffix_id = string.suffixId();
   const vector::VariableDataRegistry::DataList suffix_chunks = variable_string_data.get(suffix_id);
   const vector::VariableDataRegistry::DataList* current_chunk = &suffix_chunks;
   while (current_chunk) {
      result += current_chunk->data;
      current_chunk = current_chunk->continuation.get();
   }
   return result;
}

StringColumn::StringColumn(StringColumnMetadata* metadata)
    : metadata(metadata) {}

size_t StringColumn::chunkStart(size_t chunk_idx) const {
   return chunk_idx == 0 ? 0 : chunk_end_offsets[chunk_idx - 1];
}

std::pair<size_t, size_t> StringColumn::locate(size_t row_id) const {
   // The chunk containing `row_id` is the first whose (exclusive) end offset exceeds it.
   const auto chunk_idx = static_cast<size_t>(
      std::ranges::upper_bound(chunk_end_offsets, row_id) - chunk_end_offsets.begin()
   );
   return {chunk_idx, row_id - chunkStart(chunk_idx)};
}

SiloString StringColumn::getValue(size_t row_id) const {
   const auto [chunk_idx, row_in_chunk] = locate(row_id);
   return chunks[chunk_idx].getValue(row_in_chunk);
}

std::string StringColumn::getValueString(size_t row_id) const {
   const auto [chunk_idx, row_in_chunk] = locate(row_id);
   const auto& chunk = chunks[chunk_idx];
   return chunk.lookupValue(chunk.getValue(row_in_chunk));
}

size_t StringColumn::numValues() const {
   return chunk_end_offsets.empty() ? 0 : chunk_end_offsets.back();
}

roaring::Roaring StringColumn::getDescendants(const TreeNodeId& parent) const {
   if (!metadata->phylo_tree.has_value()) {
      return {};
   }
   return metadata->phylo_tree->getDescendants(parent);
}

namespace {
// Binds every phylo-tree leaf referenced in `buffer` to its global row id (`base + i`) atomically:
// the whole buffer is validated first, and the bindings are applied only once all of them are known
// to be valid. On failure nothing is mutated, so the caller can treat the tree as unchanged.
std::expected<void, std::string> registerPhyloNodes(
   const StringColumn::Buffer& buffer,
   size_t base,
   StringColumnMetadata* metadata
) {
   if (!metadata->phylo_tree.has_value()) {
      return {};
   }
   std::vector<std::pair<common::TreeNode*, size_t>> pending_bindings;
   // Tracks nodes already claimed earlier in this same buffer; `rowIndexExists()` cannot catch
   // these because the validated bindings have not been applied to the tree yet.
   std::unordered_set<common::TreeNode*> claimed_in_buffer;
   for (size_t i = 0; i < buffer.size(); ++i) {
      const auto& value = buffer[i];
      if (!value.has_value()) {
         continue;
      }
      auto child_it = (metadata->phylo_tree->nodes).find(TreeNodeId{*value});
      if (child_it == metadata->phylo_tree->nodes.end()) {
         continue;
      }
      common::TreeNode* node = child_it->second.get();
      if (node->rowIndexExists() || !claimed_in_buffer.insert(node).second) {
         return std::unexpected(
            fmt::format("Node '{}' already exists in the phylogenetic tree.", *value)
         );
      }
      pending_bindings.emplace_back(node, base + i);
   }
   // All bindings validated; apply them. This loop cannot fail.
   for (const auto& [node, row_id] : pending_bindings) {
      node->row_index = row_id;
   }
   return {};
}
}  // namespace

std::expected<void, std::string> StringColumn::appendChunk(const Buffer& buffer) {
   // Build the chunk in isolation so that previously appended chunks are never touched.
   // `registerPhyloNodes` is the only fallible step and applies its tree mutations atomically, so
   // it runs before any change to `null_bitmap`/`chunks`; on failure the column stays unmodified.
   const size_t base = numValues();
   if (auto result = registerPhyloNodes(buffer, base, metadata); !result.has_value()) {
      return result;
   }
   StringColumnChunk chunk;
   for (size_t i = 0; i < buffer.size(); ++i) {
      const auto& value = buffer[i];
      if (value.has_value()) {
         chunk.insert(*value);
      } else {
         null_bitmap.add(base + i);
         chunk.insertNull();
      }
   }
   chunk_end_offsets.push_back(base + chunk.numValues());
   chunks.push_back(std::move(chunk));
   return {};
}

bool StringColumn::isNull(size_t row_id) const {
   return null_bitmap.contains(row_id);
}

void StringColumnBuilder::insert(std::string_view value) {
   buffer.emplace_back(std::string{value});
}

void StringColumnBuilder::insertNull() {
   buffer.emplace_back(std::nullopt);
}

size_t StringColumnBuilder::numValues() const {
   return buffer.size();
}

StringColumn::Buffer StringColumnBuilder::finalize() {
   StringColumn::Buffer result = std::move(buffer);
   buffer.clear();
   return result;
}

}  // namespace silo::storage::column
