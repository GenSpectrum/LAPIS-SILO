#include "silo/preprocessing/phylo_tree_file.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {

TreeNode parse_auspice_tree(
   const nlohmann::json& j,
   std::optional<std::string> parent,
   std::unordered_map<std::string, TreeNode>& node_map,
   int depth = 0
) {
   TreeNode node;
   node.node_id = j.at("name").get<std::string>();
   node.parent = parent;
   node.depth = depth;

   for (const auto& child : j.at("children")) {
      TreeNode child_node = parse_auspice_tree(child, node.node_id, node_map, depth + 1);
      node.children.push_back(child_node.node_id);
   }

   // Insert node into the map *after* children so it's fully constructed
   node_map[node.node_id] = node;
   return node;
}

PhyloTreeFile PhyloTreeFile::fromAuspiceJSONString(const std::string& json_string) {
   nlohmann::json j = nlohmann::json::parse(json_string);

   PhyloTreeFile file;

   std::unordered_map<std::string, TreeNode> node_map;
   TreeNode root = parse_auspice_tree(j["tree"], std::nullopt, node_map);

   file.nodes = std::move(node_map);
   return file;
}

PhyloTreeFile PhyloTreeFile::fromAuspiceJSONFile(const std::filesystem::path& json_path) {
   std::ifstream file(json_path, std::ios::in | std::ios::binary);
   if (!file) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Could not open the JSON file: '{}'", json_path.string())
      );
   }

   std::ostringstream contents;
   if (file.peek() != std::ifstream::traits_type::eof()) {
      contents << file.rdbuf();
      if (contents.fail()) {
         throw silo::preprocessing::PreprocessingException(
            fmt::format("Error when reading the JSON file: '{}'", json_path.string())
         );
      }
   }
   try {
      return fromAuspiceJSONString(contents.str());
   } catch (const nlohmann::json::parse_error& parse_exception) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("The JSON file '{}' does not contain valid JSON.", json_path.string())
      );
   }
}

}  // namespace silo::preprocessing