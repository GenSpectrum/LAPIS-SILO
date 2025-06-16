#pragma once

#include <filesystem>
#include <vector>

#include <nlohmann/json.hpp>

namespace silo::preprocessing {

class TreeNode {
  public:
   std::string node_id;
   std::vector<std::string> children;
   std::optional<std::string> parent;
   int depth;
};

class PhyloTreeFile {
  public:
   std::unordered_map<std::string, TreeNode> nodes;

   static PhyloTreeFile fromAuspiceJSONFile(const std::filesystem::path& json_path);

   static PhyloTreeFile fromAuspiceJSONString(const std::string& json_string);
};

}  // namespace silo::preprocessing