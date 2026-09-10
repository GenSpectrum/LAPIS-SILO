#include "rhydb/query_engine/operators/order_by_with_limit_node.h"

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/compute/api.h>
#include <arrow/compute/ordering.h>
#include <nlohmann/json.hpp>

#include "rhydb/common/panic.h"
#include "rhydb/query_engine/exec_node/select_k.h"
#include "rhydb/query_engine/operators/order_by_randomize.h"
#include "rhydb/schema/database_schema.h"
#include "rhydb/storage/table.h"

namespace rhydb::query_engine::operators {

OrderByWithLimitNode::OrderByWithLimitNode(
   QueryNodePtr child,
   std::vector<OrderByField> fields,
   uint32_t limit,
   std::optional<uint32_t> offset,
   std::optional<uint32_t> randomize_seed
)
    : child(std::move(child)),
      fields(std::move(fields)),
      limit(limit),
      offset(offset),
      randomize_seed(randomize_seed) {}

std::vector<schema::ColumnIdentifier> OrderByWithLimitNode::getOutputSchema() const {
   return child->getOutputSchema();
}

namespace {

std::vector<arrow::compute::SortKey> buildSortKeys(
   const std::vector<OrderByField>& fields,
   bool has_randomize_seed
) {
   using arrow::compute::NullPlacement;
   using arrow::compute::SortOrder;

   // Build the sort keys exactly as OrderByNode does, so a rewritten query orders identically to
   // the `order_by | limit` it replaces. Nulls sort as the smallest element.
   std::vector<arrow::compute::SortKey> sort_keys;
   sort_keys.reserve(fields.size() + (has_randomize_seed ? 1 : 0));
   for (const auto& order_by_field : fields) {
      const auto sort_order =
         order_by_field.ascending ? SortOrder::Ascending : SortOrder::Descending;
      const auto null_placement =
         order_by_field.ascending ? NullPlacement::AtStart : NullPlacement::AtEnd;
      sort_keys.emplace_back(order_by_field.field.name, sort_order, null_placement);
   }
   // A randomize seed adds the per-row random hash as the lowest-priority (tie-breaking) sort key,
   // matching OrderByNode; with no explicit fields it becomes the sole key, yielding a random
   // order.
   if (has_randomize_seed) {
      sort_keys.emplace_back(RANDOMIZE_HASH_FIELD_NAME);
   }
   return sort_keys;
}

}  // namespace

arrow::Result<arrow::acero::ExecNode*> OrderByWithLimitNode::addToExecPlan(
   arrow::acero::ExecPlan& plan,
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   // The rewrite pass only produces this node when there is something to order by: sort fields, a
   // randomize seed, or both.
   RHYDB_ASSERT(!fields.empty() || randomize_seed.has_value());

   ARROW_ASSIGN_OR_RAISE(auto* top_node, child->addToExecPlan(plan, tables, query_options));

   const arrow::Ordering ordering{buildSortKeys(fields, randomize_seed.has_value())};

   if (randomize_seed.has_value()) {
      ARROW_ASSIGN_OR_RAISE(top_node, addRandomizeColumn(plan, top_node, randomize_seed.value()));
   }

   // The top-k stage preserves its input columns, so when randomizing its output still carries the
   // random-hash column; `removeRandomizeColumn` projects it back out afterwards.
   ARROW_ASSIGN_OR_RAISE(
      top_node, exec_node::addSelectKNode(plan, top_node, ordering, offset.value_or(0), limit)
   );

   if (randomize_seed.has_value()) {
      ARROW_ASSIGN_OR_RAISE(top_node, removeRandomizeColumn(plan, top_node));
   }

   return top_node;
}

nlohmann::json OrderByWithLimitNode::toJson() const {
   nlohmann::json fields_json = nlohmann::json::array();
   for (const auto& order_by_field : fields) {
      fields_json.push_back({
         {"field", columnToJson(order_by_field.field)},
         {"ascending", order_by_field.ascending},
      });
   }
   nlohmann::json result{
      {"type", nodeKindToString(kind())},
      {"fields", std::move(fields_json)},
      {"limit", limit},
      {"child", child->toJson()},
   };
   if (offset.has_value()) {
      result["offset"] = offset.value();
   }
   if (randomize_seed.has_value()) {
      result["randomizeSeed"] = randomize_seed.value();
   }
   return result;
}

}  // namespace rhydb::query_engine::operators
