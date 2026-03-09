#pragma once

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::operators {

class ZstdDecompressNode : public QueryNode {
   QueryNodePtr child;
   std::map<schema::ColumnIdentifier, std::shared_ptr<schema::TableSchema>>
      table_schemas_for_decompression;

  public:
   ZstdDecompressNode(
      QueryNodePtr child,
      std::map<schema::ColumnIdentifier, std::shared_ptr<schema::TableSchema>>
         table_schemas_for_decompression
   )
       : child(std::move(child)),
         table_schemas_for_decompression(std::move(table_schemas_for_decompression)) {}

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;
};

}  // namespace silo::query_engine::operators
