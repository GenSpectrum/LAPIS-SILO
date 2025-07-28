#pragma once

#include <memory>
#include <string>
#include <vector>

#include <arrow/result.h>

#include <nlohmann/json_fwd.hpp>

#include "silo/query_engine/actions/tree_action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/storage/table.h"

namespace silo::query_engine::actions {

class MostRecentCommonAncestor : public TreeAction {
  public:
   MostRecentCommonAncestor(std::string column_name, bool print_nodes_not_in_tree);

   arrow::Status addResponseToBuilder(
      std::vector<std::string>& all_node_ids,
      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder,
      const common::PhyloTree& phylo_tree,
      bool print_nodes_not_in_tree
   ) const override;

   std::vector<schema::ColumnIdentifier> getOutputSchema(const schema::TableSchema& table_schema
   ) const override;

   std::string_view getType() const override { return "MostRecentCommonAncestor"; }

   std::string_view myResultFieldName() const override { return "mrcaNode"; }
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<MostRecentCommonAncestor>& action);

}  // namespace silo::query_engine::actions