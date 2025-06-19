#include "silo/preprocessing/phylo_tree_file.h"

#include <fstream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "silo/preprocessing/preprocessing_exception.h"

namespace silo::preprocessing {
using silo::common::TreeNodeId;

std::shared_ptr<TreeNode> parse_auspice_tree(
   const nlohmann::json& j,
   std::optional<std::shared_ptr<TreeNode>> parent,
   std::unordered_map<TreeNodeId, std::shared_ptr<TreeNode>>& node_map,
   int depth = 0
) {
   auto node = std::make_shared<TreeNode>();
   if (!j.contains("name")) {
      throw silo::preprocessing::PreprocessingException(
         "Invalid File: Auspice JSON node does not contain a 'name' entry."
      );
   }
   node->node_id = TreeNodeId{j.at("name").get<std::string>()};
   node->parent = parent;
   node->depth = depth;

   const auto& children = j.contains("children") ? j["children"] : nlohmann::json::array();

   for (const auto& child : children) {
      auto child_node = parse_auspice_tree(child, node, node_map, depth + 1);
      node->children.push_back(child_node);
   }

   // Insert node into the map *after* children so it's fully constructed
   node_map[node->node_id] = node;
   return node;
}

PhyloTreeFile PhyloTreeFile::fromAuspiceJSONString(const std::string& json_string) {
   nlohmann::json j = nlohmann::json::parse(json_string);

   if (!j.contains("tree")) {
      throw silo::preprocessing::PreprocessingException(
         "Invalid File: Auspice JSON does not contain a 'tree' entry."
      );
   }

   PhyloTreeFile file;
   auto root = parse_auspice_tree(j["tree"], std::nullopt, file.nodes);
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

bool isValidLabelChar(char c) {
   return isalnum(c) || c == '_' || c == '.' || c == '-';
}

double parseBranchLength(std::string_view& sv) {
   std::string number;
   while (!sv.empty() && (isdigit(sv.front()) || sv.front() == '.' || sv.front() == '-' ||
                          sv.front() == '+' || sv.front() == 'e')) {
      number += sv.front();
      sv.remove_prefix(1);
   }
   return number.empty() ? 0.0 : std::stod(number);
}

TreeNodeId parseLabel(std::string_view& sv) {
   std::string label;
   while (!sv.empty() && isValidLabelChar(sv.front())) {
      label += sv.front();
      sv.remove_prefix(1);
   }
   if (!sv.empty() && sv.front() == ':') {
      sv.remove_prefix(1);
      parseBranchLength(sv);
   }
   return TreeNodeId{label};
}

void skipWhitespace(std::string_view& sv) {
   while (!sv.empty() && std::isspace(sv.front())) {
      sv.remove_prefix(1);
   }
}

std::shared_ptr<TreeNode> parseSubtree(
   std::string_view& sv,
   std::unordered_map<TreeNodeId, std::shared_ptr<TreeNode>>& node_map,
   int depth = 0,
   std::optional<std::shared_ptr<TreeNode>> parent = std::nullopt
) {
   auto node = std::make_shared<TreeNode>();
   node->depth = depth;
   node->parent = parent;

   skipWhitespace(sv);
   if (!sv.empty() && sv.front() == '(') {
      sv.remove_prefix(1);
      depth++;
      do {
         auto child_node = parseSubtree(sv, node_map, depth, node);
         node->children.push_back(child_node);
         skipWhitespace(sv);
         if (!sv.empty() && sv.front() == ',') {
            sv.remove_prefix(1);
         }
      } while (!sv.empty() && sv.front() != ')');
      if (!sv.empty() && sv.front() == ')') {
         sv.remove_prefix(1);
         depth--;
      }
   }

   if (depth != node->depth) {
      throw silo::preprocessing::PreprocessingException(
         "Parenthesis mismatch in Newick string - depth does not match"
      );
   }

   skipWhitespace(sv);
   node->node_id = parseLabel(sv);
   skipWhitespace(sv);

   node_map[node->node_id] = node;

   return node;
}

PhyloTreeFile PhyloTreeFile::fromNewickString(const std::string& newick_string) {
   PhyloTreeFile file;

   std::string_view sv(newick_string);
   if (sv.empty()) {
      throw silo::preprocessing::PreprocessingException(
         "Error when parsing the Newick string - The string is empty"
      );
   }
   if (sv.back() != ';') {
      throw silo::preprocessing::PreprocessingException(fmt::format(
         "Error when parsing the Newick string: '{}' - string does not end in ';'", newick_string
      ));
   }
   sv.remove_suffix(1);
   try {
      auto root = parseSubtree(sv, file.nodes, 0);
      if (!sv.empty()) {
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "Error when parsing the Newick string: '{}' - extra characters found", newick_string
         ));
      }
   } catch (const std::exception& e) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Error when parsing the Newick string: '{}'", newick_string)
      );
   }

   return file;
}

PhyloTreeFile PhyloTreeFile::fromNewickFile(const std::filesystem::path& newick_path) {
   std::ifstream file(newick_path, std::ios::in | std::ios::binary);
   if (!file) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Could not open the Newick file: '{}'", newick_path.string())
      );
   }

   std::ostringstream contents;
   if (file.peek() != std::ifstream::traits_type::eof()) {
      contents << file.rdbuf();
      if (contents.fail()) {
         throw silo::preprocessing::PreprocessingException(
            fmt::format("Error when reading the Newick file: '{}'", newick_path.string())
         );
      }
   }
   return fromNewickString(contents.str());
}

PhyloTreeFile PhyloTreeFile::fromFile(const std::filesystem::path& path) {
   auto ext = path.extension().string();

   std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

   if (ext != ".nwk" && ext != ".json") {
      throw std::invalid_argument("Path must end with .nwk or .json");
   }
   if (ext == ".nwk") {
      return preprocessing::PhyloTreeFile::fromNewickFile(path);
   } else if (ext == ".json") {
      return preprocessing::PhyloTreeFile::fromAuspiceJSONFile(path);
   }
}

}  // namespace silo::preprocessing