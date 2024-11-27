#include "silo/common/lineage_tree.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>

#include "silo/common/panic.h"
#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::common {

namespace {

std::string edgesToString(
   const std::vector<Idx>& ids,
   const BidirectionalMap<std::string>& lookup
) {
   std::ostringstream oss;
   for (size_t i = 0; i < ids.size(); ++i) {
      oss << lookup.getValue(ids[i]);
      if (i != ids.size() - 1) {
         oss << " -> ";
      }
   }
   return oss.str();
}

class Graph {
   size_t number_of_vertices;
   std::vector<std::vector<Idx>> adjacency_list;

   std::optional<std::vector<Idx>> findCycleWitnessFromStartVertex(
      Idx start_vertex,
      std::vector<bool>& visited
   ) const;

  public:
   [[nodiscard]] std::optional<std::vector<Idx>> getCycle() const;

   explicit Graph(size_t number_of_vertices);
   void addEdge(Idx vertex_from, Idx vertex_to);
};

Graph::Graph(size_t number_of_vertices)
    : number_of_vertices(number_of_vertices),
      adjacency_list(number_of_vertices) {}

void Graph::addEdge(Idx vertex_from, Idx vertex_to) {
   SILO_ASSERT_LT(vertex_from, number_of_vertices);
   SILO_ASSERT_LT(vertex_to, number_of_vertices);
   adjacency_list.at(vertex_from).emplace_back(vertex_to);
}

std::optional<std::vector<Idx>> Graph::findCycleWitnessFromStartVertex(
   Idx start_vertex,
   std::vector<bool>& visited
) const {
   std::vector<Idx> recursion_stack;
   std::vector<bool> in_recursion_stack(number_of_vertices);

   recursion_stack.emplace_back(start_vertex);
   in_recursion_stack[start_vertex] = true;
   visited[start_vertex] = true;

   while (!recursion_stack.empty()) {
      const Idx current_vertex = recursion_stack.back();

      bool backtrack = true;

      for (const Idx neighbor : adjacency_list[current_vertex]) {
         if (in_recursion_stack[neighbor]) {
            // We found a cycle, "visit" the vertex by immediately adding it to the stack
            // as the witness and return the witness lasso
            recursion_stack.emplace_back(neighbor);
            return recursion_stack;
         }
         if (!visited[neighbor]) {
            backtrack = false;
            visited[neighbor] = true;
            recursion_stack.emplace_back(neighbor);
            in_recursion_stack[neighbor] = true;
            break;
         }
      }

      if (backtrack) {
         recursion_stack.pop_back();
         in_recursion_stack[current_vertex] = false;
      }
   }
   return std::nullopt;
}

std::optional<std::vector<Idx>> Graph::getCycle() const {
   std::vector<bool> visited(number_of_vertices, false);

   for (Idx i = 0; i < number_of_vertices; i++) {
      if (!visited[i]) {
         auto witness_lasso = findCycleWitnessFromStartVertex(i, visited);
         if (witness_lasso.has_value()) {
            // We found a witness lasso of the form 1 -> 2 -> 3 -> 4 -> 5 -> 3
            // We need to remove leading vertices up until the cycle
            SILO_ASSERT_GE(witness_lasso.value().size(), 2UL);
            const Idx cycle_node = witness_lasso.value().back();
            auto cycle_node_first_occurrence =
               std::find(witness_lasso.value().begin(), witness_lasso.value().end(), cycle_node);
            SILO_ASSERT(cycle_node_first_occurrence < witness_lasso.value().end());
            witness_lasso.value().erase(witness_lasso.value().begin(), cycle_node_first_occurrence);
            return witness_lasso;
         }
      }
   }
   return std::nullopt;
}

}  // namespace

std::optional<std::vector<Idx>> containsCycle(
   size_t number_of_vertices,
   const std::vector<std::pair<Idx, Idx>>& edges
) {
   Graph graph(number_of_vertices);
   for (const auto& [from, to] : edges) {
      graph.addEdge(from, to);
   }
   return graph.getCycle();
}

std::optional<Idx> LineageTree::getParent(silo::Idx value_id) const {
   // TODO(#589) Recombinant lineage not yet supported -> do not follow their edges
   if (parent_relation.at(value_id).size() != 1) {
      return std::nullopt;
   }
   return parent_relation.at(value_id).at(0);
}

Idx LineageTree::resolveAlias(Idx value_id) const {
   if (alias_mapping.contains(value_id)) {
      return alias_mapping.at(value_id);
   }
   return value_id;
}

LineageTree LineageTree::fromEdgeList(
   size_t n_vertices,
   const std::vector<std::pair<Idx, Idx>>& edge_list,
   const BidirectionalMap<std::string>& lookup,
   std::unordered_map<Idx, Idx>&& alias_mapping
) {
   LineageTree result;
   result.alias_mapping = alias_mapping;
   if (auto cycle = containsCycle(n_vertices, edge_list)) {
      throw preprocessing::PreprocessingException(fmt::format(
         "The given LineageTree contains the cycle: {}", edgesToString(cycle.value(), lookup)
      ));
   }
   result.parent_relation.resize(n_vertices);
   for (const auto& [parent_id, vertex_id] : edge_list) {
      result.parent_relation.at(vertex_id).emplace_back(parent_id);
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

namespace {

void assignLineageIds(
   const preprocessing::LineageDefinitionFile& file,
   BidirectionalMap<std::string>& lookup
) {
   for (const auto& lineage : file.lineages) {
      if (lookup.getId(lineage.lineage_name.string).has_value()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "The lineage definitions contain the duplicate lineage '{}'", lineage.lineage_name
         ));
      }
      lookup.getOrCreateId(lineage.lineage_name.string);
   }
}

std::unordered_map<Idx, Idx> assignAliasIdsAndGetAliasMapping(
   const preprocessing::LineageDefinitionFile& file,
   BidirectionalMap<std::string>& lookup
) {
   std::unordered_map<Idx, Idx> alias_mapping;
   for (const auto& lineage : file.lineages) {
      const auto lineage_id = lookup.getId(lineage.lineage_name.string);
      SILO_ASSERT(lineage_id.has_value());
      for (const auto& alias : lineage.aliases) {
         if (lookup.getId(alias.string).has_value()) {
            throw silo::preprocessing::PreprocessingException(fmt::format(
               "The alias '{}' for lineage '{}' is already defined as a lineage or another alias.",
               alias,
               lineage.lineage_name
            ));
         }
         auto alias_id = lookup.getOrCreateId(alias.string);
         alias_mapping[alias_id] = lineage_id.value();
      }
   }
   return alias_mapping;
}

std::vector<std::pair<Idx, Idx>> getParentChildEdges(
   const preprocessing::LineageDefinitionFile& file,
   const BidirectionalMap<std::string>& lookup,
   const std::unordered_map<Idx, Idx>& alias_mapping
) {
   std::vector<std::pair<Idx, Idx>> edge_list;
   for (const auto& lineage : file.lineages) {
      const auto child_id = lookup.getId(lineage.lineage_name.string);
      SILO_ASSERT(child_id.has_value());

      for (const auto& parent_lineage : lineage.parents) {
         auto parent_id = lookup.getId(parent_lineage.string);
         if (!parent_id.has_value()) {
            throw preprocessing::PreprocessingException(fmt::format(
               "The lineage '{}' which is specified as the parent of vertex '{}' does not have a "
               "definition itself.",
               parent_lineage,
               lineage.lineage_name
            ));
         }
         if (alias_mapping.contains(parent_id.value())) {
            parent_id = alias_mapping.at(parent_id.value());
         }
         edge_list.emplace_back(parent_id.value(), child_id.value());
      }
   }
   return edge_list;
}

}  // namespace

LineageTreeAndIdMap LineageTreeAndIdMap::fromLineageDefinitionFile(
   const preprocessing::LineageDefinitionFile& file
) {
   BidirectionalMap<std::string> lookup;
   assignLineageIds(file, lookup);
   std::unordered_map<Idx, Idx> alias_mapping = assignAliasIdsAndGetAliasMapping(file, lookup);

   const std::vector<std::pair<Idx, Idx>> edge_list =
      getParentChildEdges(file, lookup, alias_mapping);
   auto lineage_tree =
      LineageTree::fromEdgeList(file.lineages.size(), edge_list, lookup, std::move(alias_mapping));
   return {std::move(lineage_tree), std::move(lookup)};
}

LineageTreeAndIdMap LineageTreeAndIdMap::fromLineageDefinitionFilePath(
   const std::filesystem::path& file_path
) {
   auto definition_file = preprocessing::LineageDefinitionFile::fromYAMLFile(file_path);

   return fromLineageDefinitionFile(definition_file);
}

}  // namespace silo::common
