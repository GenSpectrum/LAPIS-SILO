#include "silo/query_engine/actions/most_recent_common_ancestor.h"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/ranges.h>

#include "silo/common/phylo_tree.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::actions {
using silo::common::MRCAResponse;
using silo::common::PhyloTree;

MostRecentCommonAncestor::MostRecentCommonAncestor(
   std::string column_name,
   bool print_nodes_not_in_tree
)
    : TreeAction(std::move(column_name), print_nodes_not_in_tree) {}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Status MostRecentCommonAncestor::addResponseToBuilder(
   NodeValuesResponse& all_node_ids,
   std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder,
   const PhyloTree& phylo_tree
) const {
   MRCAResponse response = phylo_tree.getMRCA(all_node_ids.node_values);
   const std::optional<std::string> mrca_node =
      response.mrca_node_id.transform([](const auto& node_id) { return node_id.string; });
   const std::optional<std::string> mrca_parent =
      response.parent_id_of_mrca.transform([](const auto& node_id) { return node_id.string; });

   auto missing_node_count =
      static_cast<int32_t>(all_node_ids.missing_node_count + response.not_in_tree.size());

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
   const schema::TableSchema& /*table_schema*/
) const {
   auto base = makeBaseOutputSchema();
   base.emplace_back("mrcaNode", schema::ColumnType::STRING);
   base.emplace_back("mrcaParent", schema::ColumnType::STRING);
   base.emplace_back("mrcaDepth", schema::ColumnType::INT32);
   return base;
}

}  // namespace silo::query_engine::actions
