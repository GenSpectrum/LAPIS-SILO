#pragma once

#include <filesystem>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/tree_node_id.h"
#include "silo/query_engine/batched_bitmap_reader.h"

namespace silo::common {
using silo::common::TreeNodeId;

class TreeNode {
  public:
   TreeNodeId node_id;
   std::optional<size_t> row_index;  // index of corresponding sequence in the database (will be
                                     // empty for internal nodes)
   std::vector<std::shared_ptr<TreeNode>> children;
   std::optional<std::shared_ptr<TreeNode>> parent;
   int depth;

   bool isLeaf() { return children.empty(); }
   bool rowIndexExists() const { return row_index.has_value(); }

   virtual ~TreeNode() = default;

   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
    archive & node_id;
    archive & row_index;
    archive & children;
    archive & parent;
    archive & depth;
      // clang-format on
   }
};

class PhyloTree {
  public:
   std::unordered_map<TreeNodeId, std::shared_ptr<TreeNode>> nodes;

   // Functions for reading and parsing phylogenetic trees

   static PhyloTree fromAuspiceJSONFile(const std::filesystem::path& json_path);

   static PhyloTree fromAuspiceJSONString(const std::string& json_string);

   static PhyloTree fromNewickFile(const std::filesystem::path& newick_path);

   static PhyloTree fromNewickString(const std::string& newick_string);

   static PhyloTree fromFile(const std::filesystem::path& path);

   // Functions for querying the phylogenetic tree

   void validateNodeExists(const TreeNodeId& node_id);

   void validateNodeExists(const std::string& node_label);

   // returns a bitmap of all descendants node{node_id} that are also in the database
   roaring::Roaring getDescendants(const TreeNodeId& node_id);

   roaring::Roaring getDescendants(const std::string& node_label);

   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
      archive & nodes;
      // clang-format on
   }
};

}  // namespace silo::common