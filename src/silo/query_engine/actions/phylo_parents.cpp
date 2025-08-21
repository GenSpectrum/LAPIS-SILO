#include "silo/query_engine/actions/phylo_parents.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/ranges.h>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/common/phylo_tree.h"
#include "silo/common/tree_node_id.h"
#include "silo/config/database_config.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column_group.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {
using silo::common::ParentResponse;
using silo::common::PhyloTree;
using silo::common::TreeNodeId;
using silo::schema::ColumnType;

PhyloParents::PhyloParents(std::string column_name, bool print_nodes_not_in_tree)
    : TreeAction(std::move(column_name), print_nodes_not_in_tree) {}

using silo::query_engine::filter::operators::Operator;

std::optional<std::string> format_parent_node_ids(
   const std::unordered_set<std::optional<TreeNodeId>>& parent_node_ids
) {
   std::vector<TreeNodeId> valid_ids;
   for (const auto& id : parent_node_ids) {
      if (id.has_value()) {
         valid_ids.push_back(id.value());
      }
   }

   if (valid_ids.empty()) {
      return std::nullopt;
   }

   return fmt::format("{}", fmt::join(valid_ids, ","));
}

arrow::Status PhyloParents::addResponseToBuilder(
   std::unordered_set<std::string>& all_node_ids,
   std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder,
   const PhyloTree& phylo_tree,
   bool print_nodes_not_in_tree
) const {
   ParentResponse response = phylo_tree.getParents(all_node_ids);
   std::optional<std::string> parent_nodes = format_parent_node_ids(response.parent_node_ids);

   if (auto builder = output_builder.find("parentNodes"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(builder->second.insert(parent_nodes));
   }
   if (auto builder = output_builder.find("missingNodeCount"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(builder->second.insert(static_cast<int32_t>(response.not_in_tree.size()))
      );
   }
   if (auto builder = output_builder.find("missingFromTree"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(
         builder->second.insert(fmt::format("{}", fmt::join(response.not_in_tree, ",")))
      );
   }
   return arrow::Status::OK();
}

std::vector<schema::ColumnIdentifier> PhyloParents::getOutputSchema(
   const schema::TableSchema& table_schema
) const {
   auto base = makeBaseOutputSchema();
   base.emplace_back("parentNodes", schema::ColumnType::STRING);
   return base;
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PhyloParents>& action) {
   CHECK_SILO_QUERY(
      json.contains("columnName"), "error: 'columnName' field is required in PhyloParents action"
   );
   CHECK_SILO_QUERY(
      json["columnName"].is_string(),
      "error: 'columnName' field in PhyloParents action must be a string"
   );
   if (json.contains("printNodesNotInTree")) {
      CHECK_SILO_QUERY(
         json["printNodesNotInTree"].is_boolean(),
         "error: 'printNodesNotInTree' field in PhyloParents action must be a boolean"
      );
   }
   bool print_nodes_not_in_tree = json.value("printNodesNotInTree", false);
   std::string column_name = json["columnName"].get<std::string>();

   action = std::make_unique<PhyloParents>(column_name, print_nodes_not_in_tree);
}

}  // namespace silo::query_engine::actions
