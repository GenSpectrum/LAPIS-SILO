#pragma once

#include <memory>
#include <string>
#include <vector>

#include <arrow/result.h>

#include <nlohmann/json_fwd.hpp>

#include <arrow/compute/exec.h>
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/storage/table.h"

#include <silo/query_engine/bad_request.h>
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"

namespace silo::query_engine::actions {

class NodeValuesResponse {
  public:
   std::unordered_set<std::string> node_values;
   uint32_t missing_node_count = 0;
};

class TreeAction : public Action {
  private:
   std::string column_name;
   bool print_nodes_not_in_tree;

  protected:
   virtual std::string_view myResultFieldName() const = 0;

   std::vector<schema::ColumnIdentifier> makeBaseOutputSchema() const {
      std::vector<schema::ColumnIdentifier> fields;
      fields.emplace_back("missingNodeCount", schema::ColumnType::INT32);
      if (print_nodes_not_in_tree) {
         fields.emplace_back("missingFromTree", schema::ColumnType::STRING);
      }
      return fields;
   }

   void validateOrderByFields(const schema::TableSchema& schema) const override;

  public:
   TreeAction(std::string column_name, bool print_nodes_not_in_tree);

   NodeValuesResponse getNodeValues(
      std::shared_ptr<const storage::Table> table,
      const std::string& column_name,
      std::vector<CopyOnWriteBitmap>& bitmap_filter
   ) const;

   virtual arrow::Status addResponseToBuilder(
      NodeValuesResponse& all_node_ids,
      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder,
      const common::PhyloTree& phylo_tree,
      bool print_nodes_not_in_tree
   ) const = 0;

   [[nodiscard]] arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> partition_filters,
      const config::QueryOptions& query_options,
      std::string_view request_id
   ) const override;
};

}  // namespace silo::query_engine::actions
