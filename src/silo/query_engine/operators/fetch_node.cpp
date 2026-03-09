#include "silo/query_engine/operators/fetch_node.h"

#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>

#include "silo/query_engine/illegal_query_exception.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

FetchNode::FetchNode(
   QueryNodePtr child,
   std::optional<uint32_t> count,
   std::optional<uint32_t> offset
)
    : child(std::move(child)),
      count(count),
      offset(offset) {}

std::vector<schema::ColumnIdentifier> FetchNode::getOutputSchema() const {
   return child->getOutputSchema();
}

arrow::Result<PartialArrowPlan> FetchNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto partial, child->toQueryPlan(tables, query_options));
   auto* arrow_plan = partial.plan.get();
   auto* node = partial.top_node;

   CHECK_SILO_QUERY(
      !node->ordering().is_unordered(),
      "Offset and limit can only be applied if the output of the operation has some ordering. "
      "Implicit ordering such as in the case of Details/Fasta is also allowed, Aggregated "
      "however produces unordered results."
   );

   const arrow::acero::FetchNodeOptions fetch_options(
      offset.value_or(0), count.value_or(std::numeric_limits<int64_t>::max())
   );
   ARROW_ASSIGN_OR_RAISE(
      node,
      arrow::acero::MakeExecNode(
         std::string{arrow::acero::FetchNodeOptions::kName}, arrow_plan, {node}, fetch_options
      )
   );

   return PartialArrowPlan{.top_node = node, .plan = partial.plan};
}

}  // namespace silo::query_engine::operators
