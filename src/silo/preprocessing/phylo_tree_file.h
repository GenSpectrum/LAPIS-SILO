#pragma once

#include <filesystem>
#include <vector>

#include <nlohmann/json.hpp>

namespace silo::preprocessing {

class TreeNode {
  public:
   std::string node_id;
   std::vector<std::shared_ptr<TreeNode>> children;
   std::optional<std::shared_ptr<TreeNode>> parent;
   int depth;
};

class PhyloTreeFile {
  public:
   std::unordered_map<std::string, std::shared_ptr<TreeNode>> nodes;

   static PhyloTreeFile fromAuspiceJSONFile(const std::filesystem::path& json_path);

   static PhyloTreeFile fromAuspiceJSONString(const std::string& json_string);

   static PhyloTreeFile fromNewickFile(const std::filesystem::path& newick_path);

   static PhyloTreeFile fromNewickString(const std::string& newick_string);
};

}  // namespace silo::preprocessing