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
   TreeNode root = parse_auspice_tree(j["tree"], std::nullopt, file.nodes);
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

std::string parseLabel(std::string_view& sv) {
   std::string label;
   while (!sv.empty() && isValidLabelChar(sv.front())) {
      label += sv.front();
      sv.remove_prefix(1);
   }
   if (!sv.empty() && sv.front() == ':') {
      sv.remove_prefix(1);
      parseBranchLength(sv);
   }
   return label;
}

void skipWhitespace(std::string_view& sv) {
   while (!sv.empty() && std::isspace(sv.front())) {
      sv.remove_prefix(1);
   }
}

TreeNode parseSubtree(
   std::string_view& sv,
   std::unordered_map<std::string, TreeNode>& node_map,
   int depth = 0,
   std::optional<std::string> parent = std::nullopt
) {
   TreeNode node;
   node.depth = depth;
   node.parent = parent;

   skipWhitespace(sv);
   if (!sv.empty() && sv.front() == '(') {
      sv.remove_prefix(1);
      depth++;
      do {
         TreeNode child_node = parseSubtree(sv, node_map, depth, node.node_id);
         node.children.push_back(child_node.node_id);
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

   skipWhitespace(sv);
   node.node_id = parseLabel(sv);
   skipWhitespace(sv);

   node_map[node.node_id] = node;

   return node;
}

PhyloTreeFile PhyloTreeFile::fromNewickString(const std::string& newick_string) {
   PhyloTreeFile file;

   std::string_view sv(newick_string);
   try {
      auto root = parseSubtree(sv, file.nodes, 0);
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
   try {
      return fromNewickString(contents.str());
   } catch (const std::exception& e) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Error when parsing the Newick file: '{}'", newick_path.string())
      );
   }
}

}  // namespace silo::preprocessing