#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "silo/query_engine/operators/query_node.h"

namespace silo::query_engine::operators {

/// Placeholder for mostRecentCommonAncestor action, resolved during pushdown.
class UnresolvedMostRecentCommonAncestorNode final : public QueryNode {
  public:
   QueryNodePtr child;
   std::string column_name;
   bool print_nodes_not_in_tree;

   UnresolvedMostRecentCommonAncestorNode(
      QueryNodePtr child,
      std::string column_name,
      bool print_nodes_not_in_tree
   )
       : child(std::move(child)),
         column_name(std::move(column_name)),
         print_nodes_not_in_tree(print_nodes_not_in_tree) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override {
      std::vector<schema::ColumnIdentifier> output_fields;
      output_fields.emplace_back("missingNodeCount", schema::ColumnType::INT32);
      if (print_nodes_not_in_tree) {
         output_fields.emplace_back("missingFromTree", schema::ColumnType::STRING);
      }
      output_fields.emplace_back("mrcaNode", schema::ColumnType::STRING);
      output_fields.emplace_back("mrcaParent", schema::ColumnType::STRING);
      output_fields.emplace_back("mrcaDepth", schema::ColumnType::INT32);
      return output_fields;
   }

   [[nodiscard]] arrow::Result<arrow::acero::ExecNode*> addToExecPlan(
      arrow::acero::ExecPlan& /*plan*/,
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
      const config::QueryOptions& /*query_options*/
   ) const override {
      throw std::runtime_error(
         "UnresolvedMostRecentCommonAncestorNode must be eliminated during pushdown"
      );
   }

   [[nodiscard]] NodeKind kind() const override {
      return NodeKind::UNRESOLVED_MOST_RECENT_COMMON_ANCESTOR;
   }

   [[nodiscard]] nlohmann::json toJson() const override {
      return {
         {"type", nodeKindToString(kind())},
         {"columnName", column_name},
         {"printNodesNotInTree", print_nodes_not_in_tree},
         {"child", child->toJson()},
      };
   }
};

}  // namespace silo::query_engine::operators
