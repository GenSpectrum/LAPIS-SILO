#include "silo/common/phylo_tree.h"

#include <fstream>
#include <sstream>

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

#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/query_engine/batched_bitmap_reader.h"

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
   return isalnum(c) || c == '_' || c == '.' || c == '-';
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
         fmt::format("Label '{}' in Newick string contains invalid characters", fullLabel)
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

PhyloTree PhyloTree::fromNewickString(const std::string& newick_string) {
   PhyloTree file;

   std::string_view sv(newick_string);
   if (sv.empty()) {
      throw silo::preprocessing::PreprocessingException(
         "Error when parsing the Newick string - The string is empty"
      );
   }
   if (sv.back() != ';') {
      throw silo::preprocessing::PreprocessingException(
         fmt::format(
            "Error when parsing the Newick string: '{}' - string does not end in ';'", newick_string
         )
      );
   }
   sv.remove_suffix(1);
   try {
      auto root = parseSubtree(sv, file.nodes, 0);
      if (!sv.empty()) {
         throw silo::preprocessing::PreprocessingException(
            fmt::format(
               "Error when parsing the Newick string: '{}' - extra characters found", newick_string
            )
         );
      }
   } catch (const std::exception& e) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Error when parsing the Newick string '{}': {}", newick_string, e.what())
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
      throw silo::preprocessing::PreprocessingException(
         fmt::format(
            "Error when parsing the Newick string '{}': {}", newick_path.string(), e.what()
         )
      );
   }
}

PhyloTree PhyloTree::fromFile(const std::filesystem::path& path) {
   auto ext = path.extension().string();

   std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

   if (ext == ".nwk") {
      return common::PhyloTree::fromNewickFile(path);
   } else if (ext == ".json") {
      return common::PhyloTree::fromAuspiceJSONFile(path);
   }
   throw silo::preprocessing::PreprocessingException(
      fmt::format(
         "Error when parsing tree file: '{}'. Path must end with .nwk or .json", path.string()
      )
   );
}

void PhyloTree::validateNodeExists(const TreeNodeId& node_id) {
   if (nodes.find(node_id) == nodes.end()) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Node '{}' not found in the tree.", node_id.string)
      );
   }
}

void PhyloTree::validateNodeExists(const std::string& node_label) {
   auto node_id = TreeNodeId{node_label};
   validateNodeExists(node_id);
}

roaring::Roaring PhyloTree::getDescendants(const TreeNodeId& node_id) {
   validateNodeExists(node_id);
   auto child_it = nodes.find(node_id);
   roaring::Roaring result_bitmap;
   if (!child_it->second) {
      throw silo::preprocessing::PreprocessingException(
         fmt::format("Node '{}' is null.", node_id.string)
      );
   }
   std::function<void(const TreeNodeId&)> dfs = [&](const TreeNodeId& current) {
      auto current_node = nodes.find(current);
      if (!current_node->second) {
         throw silo::preprocessing::PreprocessingException(
            fmt::format("Node '{}' is null.", current.string)
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

roaring::Roaring PhyloTree::getDescendants(const std::string& node_label) {
   return getDescendants(TreeNodeId{node_label});
}

}  // namespace silo::common