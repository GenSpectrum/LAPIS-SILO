#pragma once

#include <filesystem>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/tree_node_id.h"

namespace silo::common {
using silo::common::TreeNodeId;

class TreeNode {
  public:
   TreeNodeId node_id;
   std::vector<std::shared_ptr<TreeNode>> children;
   std::optional<std::shared_ptr<TreeNode>> parent;
   int depth;
};

class PhyloTree {
  public:
   std::unordered_map<TreeNodeId, std::shared_ptr<TreeNode>> nodes;

   static PhyloTree fromAuspiceJSONFile(const std::filesystem::path& json_path);

   static PhyloTree fromAuspiceJSONString(const std::string& json_string);

   static PhyloTree fromNewickFile(const std::filesystem::path& newick_path);

   static PhyloTree fromNewickString(const std::string& newick_string);

   static PhyloTree fromFile(const std::filesystem::path& path);
};

}  // namespace silo::common