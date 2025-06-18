#pragma once

#include <filesystem>
#include <vector>

#include <nlohmann/json.hpp>

#include "silo/common/tree_node_id.h"

namespace silo::preprocessing {
using silo::common::TreeNodeId;

class TreeNode {
  public:
   TreeNodeId node_id;
   std::vector<std::shared_ptr<TreeNode>> children;
   std::optional<std::shared_ptr<TreeNode>> parent;
   int depth;
};

class PhyloTreeFile {
  public:
   std::unordered_map<TreeNodeId, std::shared_ptr<TreeNode>> nodes;

   static PhyloTreeFile fromAuspiceJSONFile(const std::filesystem::path& json_path);

   static PhyloTreeFile fromAuspiceJSONString(const std::string& json_string);

   static PhyloTreeFile fromNewickFile(const std::filesystem::path& newick_path);

   static PhyloTreeFile fromNewickString(const std::string& newick_string);

   static PhyloTreeFile fromFile(const std::filesystem::path& path);
};

}  // namespace silo::preprocessing