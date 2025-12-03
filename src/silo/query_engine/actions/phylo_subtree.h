#pragma once

#include <memory>
#include <string>
#include <vector>

#include <arrow/result.h>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/tree_action.h"

namespace silo::query_engine::actions {

class PhyloSubtree : public TreeAction {
  public:
   bool contract_unary_nodes = false;
   PhyloSubtree(std::string column_name, bool print_nodes_not_in_tree, bool contract_unary_nodes);

   arrow::Status addResponseToBuilder(
      NodeValuesResponse& all_node_ids,
      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder,
      const common::PhyloTree& phylo_tree
   ) const override;

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema(
      const schema::TableSchema& table_schema
   ) const override;

   [[nodiscard]] std::string_view getType() const override { return "PhyloSubtree"; }
   [[nodiscard]] std::string_view myResultFieldName() const override { return "subtreeNewick"; }
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<PhyloSubtree>& action);

}  // namespace silo::query_engine::actions