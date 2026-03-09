#include "silo/query_engine/actions/most_recent_common_ancestor.h"

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
using silo::common::MRCAResponse;
using silo::common::PhyloTree;

MostRecentCommonAncestor::MostRecentCommonAncestor(
   std::string column_name,
   bool print_nodes_not_in_tree
)
    : TreeAction(std::move(column_name), print_nodes_not_in_tree) {}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<MostRecentCommonAncestor>& action) {
   CHECK_SILO_QUERY(
      json.contains("columnName"),
      "error: 'columnName' field is required in MostRecentCommonAncestor action"
   );
   CHECK_SILO_QUERY(
      json["columnName"].is_string(),
      "error: 'columnName' field in MostRecentCommonAncestor action must be a string"
   );
   if (json.contains("printNodesNotInTree")) {
      CHECK_SILO_QUERY(
         json["printNodesNotInTree"].is_boolean(),
         "error: 'printNodesNotInTree' field in MostRecentCommonAncestor action must be a boolean"
      );
   }
   const bool print_nodes_not_in_tree = json.value("printNodesNotInTree", false);
   const std::string column_name = json["columnName"].get<std::string>();

   action = std::make_unique<MostRecentCommonAncestor>(column_name, print_nodes_not_in_tree);
}

}  // namespace silo::query_engine::actions
