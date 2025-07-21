#include "silo/common/phylo_tree.h"

#include <fstream>
#include <limits>
#include <set>
#include <sstream>

#include <fmt/ranges.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/query_engine/bad_request.h"

namespace silo::common {
using silo::common::TreeNodeId;

template <class Archive>
void PhyloTree::save(Archive& archive, const unsigned int version) const {
   std::vector<TreeNodeId> node_ids;
   for (const auto& [node_id, _] : nodes) {
      node_ids.push_back(node_id);
   }
   archive & node_ids;

   for (const auto& [node_id, node] : nodes) {
      auto called_node = node.get();
      SILO_ASSERT(called_node != nullptr);
      archive << *called_node;
   }
}

template <class Archive>
void PhyloTree::load(Archive& archive, const unsigned int version) {
   std::vector<TreeNodeId> node_ids;
   archive & node_ids;

   for (const auto& node_id : node_ids) {
      auto node = std::make_shared<TreeNode>();
      archive >> *node;
      nodes.emplace(node_id, std::move(node));
   }
}

template void PhyloTree::save<boost::archive::binary_oarchive>(
   boost::archive::binary_oarchive&,
   const unsigned int
) const;
template void PhyloTree::load<boost::archive::binary_iarchive>(
   boost::archive::binary_iarchive&,
   const unsigned int
);

TreeNodeId parse_auspice_tree(
   const nlohmann::json& j,
   std::optional<TreeNodeId> parent,
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
      auto child_node = parse_auspice_tree(child, node->node_id, node_map, depth + 1);
      node->children.push_back(child_node);
   }

   if (node_map.find(node->node_id) != node_map.end()) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Duplicate node ID found in Auspice JSON string: '{}'", node->node_id.string)
      );
   }

   node_map[node->node_id] = node;
   return node->node_id;
}

PhyloTree PhyloTree::fromAuspiceJSONString(const std::string& json_string) {
   nlohmann::json j = nlohmann::json::parse(json_string);

   if (!j.contains("tree")) {
      throw silo::preprocessing::PreprocessingException(
         "Invalid File: Auspice JSON does not contain a 'tree' entry."
      );
   }

   PhyloTree file;
   auto root = parse_auspice_tree(j["tree"], std::nullopt, file.nodes);
   return file;
}

PhyloTree PhyloTree::fromAuspiceJSONFile(const std::filesystem::path& json_path) {
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
   return isalnum(c) || c == '_' || c == '.' || c == '-' || c == '|' || c == '/' || c == '\\' ||
          c == '=' || c == '@';
}

bool isValidLength(char c) {
   return isdigit(c) || c == '.' || c == '-' || c == '+' || c == 'e';
}

TreeNodeId parseLabel(std::string_view& sv) {
   std::string label;
   while (!sv.empty() && isValidLabelChar(sv.back())) {
      label += sv.back();
      sv.remove_suffix(1);
   }
   std::reverse(label.begin(), label.end());
   return TreeNodeId{label};
}

TreeNodeId parseFullLabel(std::string_view& sv) {
   std::string fullLabel;
   while (!sv.empty() && (isValidLabelChar(sv.back()) || isValidLength(sv.back()))) {
      fullLabel += sv.back();
      sv.remove_suffix(1);
   }
   if (!sv.empty() && sv.back() == ':') {
      // fullLabel contains length information, we ignore it
      sv.remove_suffix(1);
      return parseLabel(sv);
   }
   if (!std::all_of(fullLabel.begin(), fullLabel.end(), isValidLabelChar)) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Label of node in Newick string contains invalid characters: '{}'", fullLabel)
      );
   }
   if (sv.back() != ')' && sv.back() != '(' && sv.back() != ',' && sv.back() != ' ') {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Newick string contains invalid characters: '{}'", sv.back())
      );
   }
   std::reverse(fullLabel.begin(), fullLabel.end());
   return TreeNodeId{fullLabel};
}

void skipWhitespace(std::string_view& sv) {
   while (!sv.empty() && std::isspace(sv.back())) {
      sv.remove_suffix(1);
   }
}

TreeNodeId parseSubtree(
   std::string_view& sv,
   std::unordered_map<TreeNodeId, std::shared_ptr<TreeNode>>& node_map,
   int depth = 0,
   std::optional<TreeNodeId> parent = std::nullopt
) {
   // We iterate through the string view from back to front inorder to know the name of the parent
   // node when we encounter a child node.
   auto node = std::make_shared<TreeNode>();
   node->depth = depth;
   node->parent = parent;

   skipWhitespace(sv);
   node->node_id = parseFullLabel(sv);
   if (!sv.empty() && sv.back() == ')') {
      sv.remove_suffix(1);
      depth++;
      do {
         auto child_node = parseSubtree(sv, node_map, depth, node->node_id);
         node->children.push_back(child_node);
         skipWhitespace(sv);
         if (!sv.empty() && sv.back() == ',') {
            sv.remove_suffix(1);
         }
      } while (!sv.empty() && sv.back() != '(');
      if (!sv.empty() && sv.back() == '(') {
         sv.remove_suffix(1);
         depth--;
      }
   }

   if (depth != node->depth) {
      throw silo::preprocessing::PreprocessingException(
         "Parenthesis mismatch in Newick string - depth does not match"
      );
   }

   skipWhitespace(sv);
   if (node_map.find(node->node_id) != node_map.end()) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Duplicate node ID found in Newick string: '{}'", node->node_id.string)
      );
   }
   node_map[node->node_id] = node;

   return node->node_id;
}

std::string_view trim(std::string_view sv) noexcept {
   constexpr auto whitespace = " \t\n\r\f\v";

   const auto start = sv.find_first_not_of(whitespace);
   if (start == std::string_view::npos) {
      return {};
   }
   const auto end = sv.find_last_not_of(whitespace);
   return sv.substr(start, end - start + 1);
}

PhyloTree PhyloTree::fromNewickString(const std::string& newick_string) {
   PhyloTree file;

   std::string_view sv(newick_string);
   sv = trim(sv);
   if (sv.empty()) {
      throw silo::preprocessing::PreprocessingException(
         "Error when parsing the Newick string - The string is empty"
      );
   }
   if (sv.back() != ';') {
      std::string shortened =
         newick_string.size() > 200 ? newick_string.substr(0, 200) + "..." : newick_string;

      throw silo::preprocessing::PreprocessingException(fmt::format(
         "Error when parsing the Newick string: '{}' - string does not end in ';'", shortened
      ));
   }
   sv.remove_suffix(1);
   try {
      auto root = parseSubtree(sv, file.nodes, 0);
      if (!sv.empty()) {
         std::string shortened =
            newick_string.size() > 200 ? newick_string.substr(0, 200) + "..." : newick_string;
         throw silo::preprocessing::PreprocessingException(fmt::format(
            "Error when parsing the Newick string: '{}' - extra characters found: '{}'",
            shortened,
            sv
         ));
      }
   } catch (const std::exception& e) {
      std::string shortened =
         newick_string.size() > 200 ? newick_string.substr(0, 200) + "..." : newick_string;
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Error when parsing the Newick string '{}': {}", shortened, e.what())
      );
   }

   return file;
}

PhyloTree PhyloTree::fromNewickFile(const std::filesystem::path& newick_path) {
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
      throw silo::preprocessing::PreprocessingException(fmt::format(
         "Error when parsing the Newick string '{}': {}", newick_path.string(), e.what()
      ));
   }
}

PhyloTree PhyloTree::fromFile(const std::filesystem::path& path) {
   EVOBENCH_SCOPE("PhyloTree", "fromFile");

   auto ext = path.extension().string();

   std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

   if (ext == ".nwk") {
      return common::PhyloTree::fromNewickFile(path);
   } else if (ext == ".json") {
      return common::PhyloTree::fromAuspiceJSONFile(path);
   }
   throw silo::preprocessing::PreprocessingException(fmt::format(
      "Error when parsing tree file: '{}'. Path must end with .nwk or .json", path.string()
   ));
}

std::optional<TreeNodeId> PhyloTree::getTreeNodeId(const std::string& node_label) {
   auto node_id = TreeNodeId{node_label};
   if (nodes.find(node_id) == nodes.end()) {
      return std::nullopt;
   }
   return node_id;
}

roaring::Roaring PhyloTree::getDescendants(const TreeNodeId& node_id) {
   auto child_it = nodes.find(node_id);
   roaring::Roaring result_bitmap;
   if (child_it == nodes.end() || !child_it->second) {
      throw std::runtime_error(
         fmt::format("Node '{}' is null - this is an internal error.", node_id.string)
      );
   }
   std::function<void(const TreeNodeId&)> dfs = [&](const TreeNodeId& current) {
      auto current_node = nodes.find(current);
      if (!current_node->second) {
         throw std::runtime_error(
            fmt::format("Node '{}' is null - this is an internal error.", current.string)
         );
      }
      if (current_node->second->isLeaf()) {
         if (current_node->second->row_index.has_value()) {
            result_bitmap.add(current_node->second->row_index.value());
         }
      }
      for (const auto& child : current_node->second->children) {
         dfs(child);
      }
   };
   if (child_it->second->isLeaf()) {
      return result_bitmap;
   }
   dfs(node_id);
   return result_bitmap;
}

void PhyloTree::getSetOfAncestorsAtDepth(
   const std::set<TreeNodeId>& nodes_to_group,
   std::set<TreeNodeId>& ancestors_at_depth,
   int depth
) const {
   for (const auto& node_id : nodes_to_group) {
      auto node_it = nodes.find(node_id);
      while (node_it != nodes.end() && node_it->second->depth > depth) {
         if (node_it->second->parent.has_value()) {
            node_it = nodes.find(node_it->second->parent.value());
         } else {
            break;
         }
      }
      if (node_it == nodes.end()) {
         throw std::runtime_error(fmt::format("Node '{}' does not exist in tree.", node_id.string));
      }
      ancestors_at_depth.insert(node_it->first);
   }
}

MRCAResponse PhyloTree::getMRCA(const std::vector<std::string>& node_labels) const {
   MRCAResponse response;
   std::set<TreeNodeId> nodes_to_group;
   int min_depth = std::numeric_limits<int>::max();

   for (const auto& node_label : node_labels) {
      auto node_it = nodes.find(TreeNodeId{node_label});
      if (node_it == nodes.end()) {
         response.not_in_tree.push_back(node_label);
      } else {
         nodes_to_group.insert(TreeNodeId{node_label});
         if (node_it->second->depth < min_depth) {
            min_depth = node_it->second->depth;
         }
      }
   }

   if (nodes_to_group.empty()) {
      response.mrca_node_id = std::nullopt;
      return response;
   }

   std::set<TreeNodeId> set_at_min_depth;
   getSetOfAncestorsAtDepth(nodes_to_group, set_at_min_depth, min_depth);

   while (set_at_min_depth.size() > 1) {
      SILO_ASSERT(min_depth > 0);
      min_depth--;
      std::set<TreeNodeId> next_set_at_min_depth;
      getSetOfAncestorsAtDepth(nodes_to_group, next_set_at_min_depth, min_depth);
      set_at_min_depth = next_set_at_min_depth;
   }
   if (set_at_min_depth.empty()) {
      throw std::runtime_error(
         "No common ancestor found for the provided nodes. This is an internal error."
      );
   }
   response.mrca_node_id = *set_at_min_depth.begin();
   return response;
}

}  // namespace silo::common