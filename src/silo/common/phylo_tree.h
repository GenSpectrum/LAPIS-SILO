#pragma once

#include <filesystem>
#include <set>
#include <unordered_set>
#include <vector>

#include <roaring/roaring.hh>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>

#include <nlohmann/json.hpp>

#include "silo/common/panic.h"
#include "silo/common/tree_node_id.h"

namespace silo::common {
using silo::common::TreeNodeId;

class TreeNode {
  public:
   TreeNodeId node_id;
   std::optional<size_t> row_index;  // index of corresponding sequence in the database (will be
                                     // empty for internal nodes)
   std::vector<TreeNodeId> children;
   std::optional<TreeNodeId> parent;
   int depth;

   bool isLeaf() { return children.empty(); }
   bool rowIndexExists() const { return row_index.has_value(); }

   friend class boost::serialization::access;
   template <class Archive>
   [[maybe_unused]] void serialize(Archive& archive, const uint32_t /* version */) {
      // clang-format off
    archive & node_id;
    archive & children;
    archive & parent;
    archive & depth;
    archive & row_index;
      // clang-format on
   }
};

class MRCAResponse {
  public:
   std::optional<TreeNodeId> mrca_node_id;
   std::vector<std::string> not_in_tree;
};

class NewickResponse {
  public:
   std::string newick_string;
   std::vector<std::string> not_in_tree;
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

   std::optional<TreeNodeId> getTreeNodeId(const std::string& node_label);

   // returns a bitmap of all descendants node{node_id} that are also in the database
   roaring::Roaring getDescendants(const TreeNodeId& node_id);

   MRCAResponse getMRCA(const std::unordered_set<std::string>& node_labels) const;

   NewickResponse toNewickString(
      const std::unordered_set<std::string>& filter,
      bool contract_unary_nodes = true
   ) const;

   std::optional<std::string> partialNewickString(
      const std::unordered_set<std::string>& filter,
      const TreeNodeId& ancestor,
      bool contract_unary_nodes = true
   ) const;

   void getSetOfAncestorsAtDepth(
      const std::set<TreeNodeId>& nodes_to_group,
      std::set<TreeNodeId>& ancestors_at_depth,
      int depth
   ) const;

  private:
   friend class boost::serialization::access;
   template <class Archive>
   void save(Archive& ar, const unsigned int version) const;

   template <class Archive>
   void load(Archive& ar, const unsigned int version);

   template <class Archive>
   void serialize(Archive& ar, const unsigned int version) {
      boost::serialization::split_member(ar, *this, version);
   }
};

}  // namespace silo::common