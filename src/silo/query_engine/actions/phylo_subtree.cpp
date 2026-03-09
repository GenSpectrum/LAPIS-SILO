#include "silo/query_engine/actions/phylo_subtree.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#include "silo/common/phylo_tree.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::actions {

PhyloSubtree::PhyloSubtree(
   std::string column_name,
   bool print_nodes_not_in_tree,
   bool contract_unary_nodes
)
    : column_name(std::move(column_name)),
      print_nodes_not_in_tree(print_nodes_not_in_tree),
      contract_unary_nodes(contract_unary_nodes) {}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PhyloSubtree>& action) {
   CHECK_SILO_QUERY(
      json.contains("columnName"), "error: 'columnName' field is required in PhyloSubtree action"
   );
   CHECK_SILO_QUERY(
      json["columnName"].is_string(),
      "error: 'columnName' field in PhyloSubtree action must be a string"
   );
   if (json.contains("printNodesNotInTree")) {
      CHECK_SILO_QUERY(
         json["printNodesNotInTree"].is_boolean(),
         "error: 'printNodesNotInTree' field in PhyloSubtree action must be a boolean"
      );
   }
   if (json.contains("contractUnaryNodes")) {
      CHECK_SILO_QUERY(
         json["contractUnaryNodes"].is_boolean(),
         "error: 'contractUnaryNodes' field in PhyloSubtree action must be a boolean"
      );
   }
   const bool print_nodes_not_in_tree = json.value("printNodesNotInTree", false);
   const bool contract_unary_nodes = json.value("contractUnaryNodes", true);
   const std::string column_name = json["columnName"].get<std::string>();

   action =
      std::make_unique<PhyloSubtree>(column_name, print_nodes_not_in_tree, contract_unary_nodes);
}

}  // namespace silo::query_engine::actions
