#include "silo/query_engine/actions/most_recent_common_ancestor.h"

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
using silo::common::MRCAResponse;
using silo::common::PhyloTree;
using silo::common::TreeNodeId;
using silo::schema::ColumnType;

MostRecentCommonAncestor::MostRecentCommonAncestor(
   std::string column_name,
   bool print_nodes_not_in_tree
)
    : TreeAction(std::move(column_name), print_nodes_not_in_tree) {}

using silo::query_engine::filter::operators::Operator;

arrow::Status MostRecentCommonAncestor::addResponseToBuilder(
   NodeValuesResponse& all_node_ids,
   std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder,
   const PhyloTree& phylo_tree,
   bool print_nodes_not_in_tree
) const {
   MRCAResponse response = phylo_tree.getMRCA(all_node_ids.node_values);
   std::optional<std::string> mrca_node =
      response.mrca_node_id.transform([](const auto& id) { return id.string; });
   std::optional<std::string> mrca_parent =
      response.parent_id_of_mrca.transform([](const auto& id) { return id.string; });

   int32_t missing_node_count =
      all_node_ids.missing_node_count + static_cast<int32_t>(response.not_in_tree.size());

   if (auto builder = output_builder.find("mrcaNode"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(builder->second.insert(mrca_node));
   }
   if (auto builder = output_builder.find("mrcaParent"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(builder->second.insert(mrca_parent));
   }
   if (auto builder = output_builder.find("mrcaDepth"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(builder->second.insert(response.mrca_depth));
   }
   if (auto builder = output_builder.find("missingNodeCount"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(builder->second.insert(missing_node_count));
   }
   if (auto builder = output_builder.find("missingFromTree"); builder != output_builder.end()) {
      ARROW_RETURN_NOT_OK(
         builder->second.insert(fmt::format("{}", fmt::join(response.not_in_tree, ",")))
      );
   }
   return arrow::Status::OK();
}

std::vector<schema::ColumnIdentifier> MostRecentCommonAncestor::getOutputSchema(
   const schema::TableSchema& table_schema
) const {
   auto base = makeBaseOutputSchema();
   base.emplace_back("mrcaNode", schema::ColumnType::STRING);
   base.emplace_back("mrcaParent", schema::ColumnType::STRING);
   base.emplace_back("mrcaDepth", schema::ColumnType::INT32);
   return base;
}

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
   bool print_nodes_not_in_tree = json.value("printNodesNotInTree", false);
   std::string column_name = json["columnName"].get<std::string>();

   action = std::make_unique<MostRecentCommonAncestor>(column_name, print_nodes_not_in_tree);
}

}  // namespace silo::query_engine::actions
