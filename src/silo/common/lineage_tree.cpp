#include "silo/common/lineage_tree.h"

#include <fmt/format.h>

#include "silo/common/panic.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::common {

using silo::preprocessing::LineageName;

namespace {

class UnionFind {
  public:
   std::vector<uint32_t> parent;
   std::vector<uint32_t> rank;

   explicit UnionFind(size_t n) {
      parent.resize(n);
      rank.resize(n, 0);
      for (uint32_t i = 0; i < n; ++i) {
         parent[i] = i;
      }
   }

   int find(uint32_t u) {
      if (u != parent[u]) {
         parent[u] = find(parent[u]);
      }
      return parent[u];
   }

   bool unionSets(int u, int v) {
      int rootU = find(u);
      int rootV = find(v);

      if (rootU == rootV)
         return false;

      if (rank[rootU] > rank[rootV]) {
         parent[rootV] = rootU;
      } else if (rank[rootU] < rank[rootV]) {
         parent[rootU] = rootV;
      } else {
         parent[rootV] = rootU;
         rank[rootU]++;
      }
      return true;
   }
};

}  // namespace

bool containsCycle(int n, const std::vector<std::pair<Idx, Idx>>& edges) {
   UnionFind union_find(n);
   for (const auto& [vertex1, vertex2] : edges) {
      if (!union_find.unionSets(vertex1, vertex2)) {
         return true;
      }
   }
   return false;
}

std::optional<Idx> LineageTree::getParent(silo::Idx value) {
   return parent_relation.at(value);
}

LineageTree LineageTree::fromEdgeList(
   size_t n_vertices,
   const std::vector<std::pair<Idx, Idx>>& edge_list
) {
   if (containsCycle(n_vertices, edge_list)) {
      throw preprocessing::PreprocessingException("The given LineageTree contains a cycle.");
   }
   LineageTree result;
   result.parent_relation.resize(n_vertices);
   for (const auto& [vertex_id, parent] : edge_list) {
      if (result.parent_relation.at(vertex_id).has_value()) {
         PANIC(
            "Implementation error. Recombinant trees not supported, but encountered a recombinant "
            "entry in edge list."
         );
      }
      result.parent_relation.at(vertex_id) = parent;
   }
   return result;
}

LineageTreeAndIdMap::LineageTreeAndIdMap(const LineageTreeAndIdMap& other)
    : lineage_tree(other.lineage_tree),
      lineage_id_lookup_map(other.lineage_id_lookup_map.copy()) {}

LineageTreeAndIdMap& LineageTreeAndIdMap::operator=(const LineageTreeAndIdMap& other) {
   lineage_tree = other.lineage_tree;
   lineage_id_lookup_map = other.lineage_id_lookup_map.copy();
   return *this;
}

LineageTreeAndIdMap::LineageTreeAndIdMap(
   LineageTree&& lineage_tree,
   BidirectionalMap<std::string>&& lineage_id_lookup_map
)
    : lineage_tree(std::move(lineage_tree)),
      lineage_id_lookup_map(std::move(lineage_id_lookup_map)) {}

LineageTreeAndIdMap LineageTreeAndIdMap::fromLineageDefinitionFile(
   const preprocessing::LineageDefinitionFile& file
) {
   BidirectionalMap<std::string> lookup;
   for (const auto& lineage : file.lineages) {
      lookup.getOrCreateId(lineage.lineage_name.string);
   }
   std::vector<std::pair<Idx, Idx>> edge_list;
   for (const auto& lineage : file.lineages) {
      const Idx my_id = lookup.getId(lineage.lineage_name.string).value();

      if (lineage.parents.size() > 1) {
         // TODO(#589) Recombinant lineage not yet supported
         continue;
      }
      if (lineage.parents.size() == 1) {
         const LineageName parent_lineage = lineage.parents.at(0);
         auto parent_id = lookup.getId(parent_lineage.string);
         if (!parent_id.has_value()) {
            throw preprocessing::PreprocessingException(fmt::format(
               "The lineage '{}' which is specified as the parent of vertex '{}' does not have a "
               "definition itself.",
               parent_lineage.string,
               lineage.lineage_name.string
            ));
         }
         edge_list.emplace_back(my_id, parent_id.value());
      }
   }
   return {LineageTree::fromEdgeList(file.lineages.size(), edge_list), std::move(lookup)};
}

LineageTreeAndIdMap LineageTreeAndIdMap::fromLineageDefinitionFilePath(
   const std::filesystem::path& file_path
) {
   auto definition_file = preprocessing::LineageDefinitionFile::fromYAMLFile(file_path);

   return fromLineageDefinitionFile(definition_file);
}

}  // namespace silo::common