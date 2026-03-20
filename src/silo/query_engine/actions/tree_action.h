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
   [[nodiscard]] virtual std::string_view myResultFieldName() const = 0;

   [[nodiscard]] std::vector<schema::ColumnIdentifier> makeBaseOutputSchema() const {
      std::vector<schema::ColumnIdentifier> fields;
      fields.emplace_back("missingNodeCount", schema::ColumnType::INT32);
      if (print_nodes_not_in_tree) {
         fields.emplace_back("missingFromTree", schema::ColumnType::STRING);
      }
      return fields;
   }

  public:
   TreeAction(std::string column_name, bool print_nodes_not_in_tree);

   [[nodiscard]] const std::string& getColumnName() const { return column_name; }
   [[nodiscard]] bool getPrintNodesNotInTree() const { return print_nodes_not_in_tree; }
};

}  // namespace silo::query_engine::actions
