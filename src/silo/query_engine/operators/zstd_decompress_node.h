#pragma once

#include <string>
#include <vector>

#include "silo/query_engine/operators/query_node.h"
#include "silo/schema/database_schema.h"

namespace silo::query_engine::operators {

class ZstdDecompressNode : public QueryNode {
  public:
   /// Describes one column that is decompressed by this node. All other child columns
   /// are forwarded unchanged. `input` is the sequence-typed child column; `output`
   /// has the same name but STRING type. `reference` is the dictionary/reference string
   /// used for decompression.
   struct ColumnMapping {
      schema::ColumnIdentifier input;
      schema::ColumnIdentifier output;
      std::string reference;
   };

   QueryNodePtr child;

   /// Only the columns that need decompression — pass-through columns are not listed here.
   std::vector<ColumnMapping> column_mapping;

   ZstdDecompressNode(QueryNodePtr child, std::vector<ColumnMapping> column_mapping);

   [[nodiscard]] std::vector<schema::ColumnIdentifier> getOutputSchema() const override;

   [[nodiscard]] arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const override;

   [[nodiscard]] NodeKind kind() const override { return NodeKind::ZSTD_DECOMPRESS; }
};

/// Builds the ColumnMapping vector from a child's output schema and the map of columns
/// that require decompression (each paired with the TableSchema that provides the
/// reference/dictionary string). Only sequence-typed columns present in `table_schemas`
/// produce an entry; all other columns are pass-through and are not listed.
std::vector<ZstdDecompressNode::ColumnMapping> buildDecompressColumnMapping(
   const std::vector<schema::ColumnIdentifier>& child_schema,
   const std::map<schema::ColumnIdentifier, std::shared_ptr<schema::TableSchema>>& table_schemas
);

}  // namespace silo::query_engine::operators
