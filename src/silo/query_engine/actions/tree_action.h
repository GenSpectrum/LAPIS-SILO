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

   std::vector<std::string> getNodeValues(
      std::shared_ptr<const storage::Table> table,
      const std::string& column_name,
      std::vector<CopyOnWriteBitmap>& bitmap_filter
   ) const;

   virtual arrow::Status addResponseToBuilder(
      std::vector<std::string>& all_node_ids,
      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder,
      const storage::column::StringColumnMetadata* metadata,
      bool print_nodes_not_in_tree
   ) const = 0;

   arrow::Result<QueryPlan> toQueryPlanImpl(
      std::shared_ptr<const storage::Table> table,
      std::vector<CopyOnWriteBitmap> partition_filters,
      const config::QueryOptions& query_options
   ) const override;
};

template <class ActionT>
std::unique_ptr<ActionT> makeTreeAction(const nlohmann::json& json, std::string_view actionName) {
   CHECK_SILO_QUERY(
      json.contains("columnName"), "error: 'columnName' field is required in {} action", actionName
   );
   CHECK_SILO_QUERY(
      json["columnName"].is_string(),
      "error: 'columnName' field in {} action must be a string",
      actionName
   );
   if (json.contains("printNodesNotInTree")) {
      CHECK_SILO_QUERY(
         json["printNodesNotInTree"].is_boolean(),
         "error: 'printNodesNotInTree' field in {} action must be a boolean",
         actionName
      );
   }

   bool print_nodes_not_in_tree = json.value("printNodesNotInTree", false);
   std::string column_name = json["columnName"].get<std::string>();

   return std::make_unique<ActionT>(std::move(column_name), print_nodes_not_in_tree);
}

}  // namespace silo::query_engine::actions
