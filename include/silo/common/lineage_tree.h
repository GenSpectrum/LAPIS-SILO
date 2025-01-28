#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/bidirectional_map.h"
#include "silo/common/lineage_name.h"
#include "silo/common/types.h"
#include "silo/preprocessing/lineage_definition_file.h"

namespace silo::common {

// The tree is allowed to be disconnected
class LineageTree {
   friend class boost::serialization::access;
   std::vector<std::vector<Idx>> parent_relation;
   std::unordered_map<Idx, Idx> alias_mapping;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & parent_relation;
      archive & alias_mapping;
      // clang-format on
   }

  public:
   LineageTree() = default;
   LineageTree(LineageTree&& other) = default;
   LineageTree(const LineageTree& other) = default;
   LineageTree& operator=(const LineageTree& other) = default;
   LineageTree& operator=(LineageTree&& other) = default;

   static LineageTree fromEdgeList(
      size_t n_vertices,
      const std::vector<std::pair<Idx, Idx>>& edge_list,
      const BidirectionalMap<std::string>& lookup,
      std::unordered_map<Idx, Idx>&& alias_mapping
   );

   std::optional<Idx> getParent(Idx value_id) const;

   Idx resolveAlias(Idx value_id) const;
};

class LineageTreeAndIdMap {
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & lineage_tree;
      archive & lineage_id_lookup_map;
      archive & file;
      // clang-format on
   }

  public:
   LineageTree lineage_tree;
   BidirectionalMap<std::string> lineage_id_lookup_map;
   std::string file;

   LineageTreeAndIdMap() = default;
   LineageTreeAndIdMap(LineageTreeAndIdMap&& other) = default;
   LineageTreeAndIdMap& operator=(LineageTreeAndIdMap&& other) = default;
   LineageTreeAndIdMap(const LineageTreeAndIdMap& other);
   LineageTreeAndIdMap& operator=(const LineageTreeAndIdMap& other);

   static LineageTreeAndIdMap fromLineageDefinitionFile(
      silo::preprocessing::LineageDefinitionFile&& file
   );

   static LineageTreeAndIdMap fromLineageDefinitionFilePath(const std::filesystem::path& file_path);

  private:
   LineageTreeAndIdMap(
      LineageTree&& lineage_tree,
      BidirectionalMap<std::string>&& lineage_id_lookup_map,
      std::string&& file
   );
};

std::optional<std::vector<Idx>> containsCycle(
   size_t number_of_vertices,
   const std::vector<std::pair<Idx, Idx>>& edges
);

}  // namespace silo::common
