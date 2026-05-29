#pragma once

#include <map>
#include <memory>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/result.h>

#include "silo/config/runtime_config.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

enum class NodeKind : uint8_t {
   AGGREGATE,
   PROJECT,
   ORDER_BY,
   FETCH,
   FILTER,
   UNRESOLVED_MUTATIONS_NUCLEOTIDE,
   UNRESOLVED_MUTATIONS_AMINO_ACID,
   UNRESOLVED_INSERTIONS_NUCLEOTIDE,
   UNRESOLVED_INSERTIONS_AMINO_ACID,
   UNRESOLVED_MOST_RECENT_COMMON_ANCESTOR,
   UNRESOLVED_PHYLO_SUBTREE,
   MUTATIONS_NUCLEOTIDE,
   MUTATIONS_AMINO_ACID,
   INSERTIONS_NUCLEOTIDE,
   INSERTIONS_AMINO_ACID,
   MOST_RECENT_COMMON_ANCESTOR,
   PHYLO_SUBTREE,
   TABLE_SCAN,
   COUNT_FILTER,
   ZSTD_DECOMPRESS,
};

struct PartialArrowPlan {
   arrow::acero::ExecNode* top_node;
   std::shared_ptr<arrow::acero::ExecPlan> plan;
};

class QueryNode {
  public:
   virtual ~QueryNode() = default;

   [[nodiscard]] virtual arrow::Result<PartialArrowPlan> toQueryPlan(
      const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
      const config::QueryOptions& query_options
   ) const = 0;

   [[nodiscard]] virtual std::vector<schema::ColumnIdentifier> getOutputSchema() const = 0;

   [[nodiscard]] virtual NodeKind kind() const = 0;
};

using QueryNodePtr = std::unique_ptr<QueryNode>;

}  // namespace silo::query_engine::operators
