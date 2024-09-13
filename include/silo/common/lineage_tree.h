#pragma once

#include <vector>

#include <boost/serialization/access.hpp>

#include "silo/common/bidirectional_map.h"
#include "silo/preprocessing/lineage_definition_file.h"

namespace silo::common {

// The tree is allowed to be disconnected
class LineageTree {
   friend class boost::serialization::access;
   std::vector<std::optional<Idx>> parent_relation;

   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & parent_relation;
      // clang-format on
   }

  public:
   LineageTree() = default;
   LineageTree(LineageTree&& other) = default;
   LineageTree(const LineageTree& other) = default;
   LineageTree& operator=(const LineageTree& other) = default;
   LineageTree& operator=(LineageTree&& other) = default;

   static LineageTree fromEdgeList(size_t n_vertices, std::vector<std::pair<Idx, Idx>> edge_list);

   std::optional<Idx> getParent(Idx value);
};

class LineageTreeAndIDMap {
   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & lineage_tree;
      archive & lineage_id_lookup_map;
      // clang-format on
   }

  public:
   LineageTree lineage_tree;
   BidirectionalMap<std::string> lineage_id_lookup_map;

   LineageTreeAndIDMap() = default;
   LineageTreeAndIDMap(LineageTreeAndIDMap&& other) = default;
   LineageTreeAndIDMap& operator=(LineageTreeAndIDMap&& other) = default;
   LineageTreeAndIDMap(const LineageTreeAndIDMap& other);
   LineageTreeAndIDMap& operator=(const LineageTreeAndIDMap& other);

   static LineageTreeAndIDMap fromLineageDefinitionFile(
      const silo::preprocessing::LineageDefinitionFile& file
   );

   static LineageTreeAndIDMap fromLineageDefinitionFilePath(const std::filesystem::path& file_path);

  private:
   LineageTreeAndIDMap(
      LineageTree&& lineage_tree,
      BidirectionalMap<std::string>&& lineage_id_lookup_map
   );
};

bool containsCycle(int n, const std::vector<std::pair<Idx, Idx>>& edges);

}  // namespace silo::common
