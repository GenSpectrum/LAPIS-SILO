#include "silo/query_engine/operators/order_by_node.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/builder.h>
#include <arrow/compute/api.h>
#include <arrow/compute/ordering.h>
#include <arrow/util/async_generator_fwd.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>

#include "silo/common/panic.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

namespace {

const std::string RANDOMIZE_HASH_FIELD_NAME{"__SILO_RANDOMIZE_HASH"};

uint64_t hash64(uint64_t value, uint64_t seed) {
   value ^= seed;
   value ^= value >> 33;
   value *= 0xff51afd7ed558ccdULL;
   value ^= value >> 33;
   value *= 0xc4ceb9fe1a85ec53ULL;
   value ^= value >> 33;
   return value;
}

arrow::Result<arrow::acero::ExecNode*> removeRandomizeColumn(const PartialArrowPlan& plan) {
   std::vector<arrow::Expression> field_refs;
   for (const auto& field : plan.top_node->output_schema()->fields()) {
      if (field->name() != RANDOMIZE_HASH_FIELD_NAME) {
         field_refs.push_back(arrow::compute::field_ref(field->name()));
      }
   }
   auto options = arrow::acero::ProjectNodeOptions(field_refs);
   return arrow::acero::MakeExecNode("project", plan.plan.get(), {plan.top_node}, options);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<arrow::acero::ExecNode*> addRandomizeColumn(
   PartialArrowPlan plan,
   size_t randomize_seed
) {
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> sequenced_batches;
   std::shared_ptr<arrow::Schema> schema_of_sequence_batches;
   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode(
         "sink",
         plan.plan.get(),
         {plan.top_node},
         arrow::acero::SinkNodeOptions{&sequenced_batches, &schema_of_sequence_batches}
      )
   );
   plan.top_node->SetLabel("input to randomize column projection");
   auto output_schema_fields = schema_of_sequence_batches->fields();
   output_schema_fields.emplace_back(
      std::make_shared<arrow::Field>(RANDOMIZE_HASH_FIELD_NAME, arrow::uint64())
   );
   auto output_schema = arrow::schema(output_schema_fields);
   size_t start_of_batch = 0;
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> sequenced_batches_with_hash_id =
      // NOLINTNEXTLINE(readability-function-cognitive-complexity)
      [sequenced_batches, start_of_batch, randomize_seed](
      ) mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      SPDLOG_TRACE("randomize column projection awaits the next batch");
      auto future = sequenced_batches();

      return future.Then(
         [&](std::optional<arrow::ExecBatch> maybe_input_batch
         ) mutable -> arrow::Result<std::optional<arrow::ExecBatch>> {
            SPDLOG_TRACE("randomize column projection received next batch");

            if (!maybe_input_batch.has_value()) {
               return std::nullopt;
            }

            const auto& input_batch = maybe_input_batch.value();
            SILO_ASSERT(!input_batch.values.empty());
            auto rows_in_batch = input_batch.values.at(0).length();
            SILO_ASSERT_NE(rows_in_batch, arrow::Datum::kUnknownLength);

            arrow::UInt64Builder randomize_column_builder;
            for (int64_t i = 0; i < rows_in_batch; ++i) {
               const uint64_t hash_val = hash64(start_of_batch + i, randomize_seed);
               ARROW_RETURN_NOT_OK(randomize_column_builder.Append(hash_val));
            }

            ARROW_ASSIGN_OR_RAISE(auto randomize_column, randomize_column_builder.Finish());
            start_of_batch += rows_in_batch;

            auto output_columns = input_batch.values;
            output_columns.emplace_back(randomize_column);
            auto output_batch = arrow::ExecBatch::Make(output_columns);
            return output_batch;
         }
      );
   };
   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode(
         "source",
         plan.plan.get(),
         {},
         arrow::acero::SourceNodeOptions{output_schema, std::move(sequenced_batches_with_hash_id)}
      )
   );
   plan.top_node->SetLabel("output of randomize column projection");
   return plan.top_node;
}

arrow::Result<arrow::acero::ExecNode*> addSortNode(
   PartialArrowPlan plan,
   const std::vector<schema::ColumnIdentifier>& output_fields,
   const arrow::Ordering& ordering
) {
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> generator;
   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode(
         "order_by_sink",
         plan.plan.get(),
         {plan.top_node},
         arrow::acero::OrderBySinkNodeOptions{arrow::SortOptions{ordering}, &generator}
      )
   );
   plan.top_node->SetLabel("order by");
   auto schema = exec_node::columnsToArrowSchema(output_fields);
   return arrow::acero::MakeExecNode(
      "source",
      plan.plan.get(),
      {},
      arrow::acero::SourceNodeOptions{schema, std::move(generator), ordering}
   );
}

}  // namespace

OrderByNode::OrderByNode(
   QueryNodePtr child,
   std::vector<OrderByField> fields,
   std::optional<uint32_t> randomize_seed
)
    : child(std::move(child)),
      fields(std::move(fields)),
      randomize_seed(randomize_seed) {}

std::vector<schema::ColumnIdentifier> OrderByNode::getOutputSchema() const {
   return child->getOutputSchema();
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<PartialArrowPlan> OrderByNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   // Validate order-by fields exist in child output schema
   auto child_schema = child->getOutputSchema();
   std::vector<std::string> field_names;
   field_names.reserve(child_schema.size());
   for (const auto& identifier : child_schema) {
      field_names.push_back(identifier.name);
   }
   for (const auto& order_by_field : fields) {
      CHECK_SILO_QUERY(
         std::ranges::find(field_names, order_by_field.name) != field_names.end(),
         "OrderByField {} is not contained in the result of this operation. "
         "Allowed values are {}.",
         order_by_field.name,
         fmt::join(field_names, ", ")
      );
   }

   ARROW_ASSIGN_OR_RAISE(auto plan, child->toQueryPlan(tables, query_options));

   using arrow::compute::NullPlacement;
   using arrow::compute::SortOrder;

   std::vector<arrow::compute::SortKey> sort_keys;
   for (const auto& order_by_field : fields) {
      auto sort_order = order_by_field.ascending ? SortOrder::Ascending : SortOrder::Descending;
      sort_keys.emplace_back(order_by_field.name, sort_order);
   }
   if (randomize_seed.has_value()) {
      sort_keys.emplace_back(RANDOMIZE_HASH_FIELD_NAME);
   }

   if (sort_keys.empty()) {
      return plan;
   }

   auto first_sort_key = sort_keys.at(0);
   const auto null_placement = first_sort_key.order == arrow::compute::SortOrder::Ascending
                                  ? NullPlacement::AtStart
                                  : NullPlacement::AtEnd;
   const arrow::Ordering ordering{sort_keys, null_placement};

   if (randomize_seed.has_value()) {
      ARROW_ASSIGN_OR_RAISE(plan.top_node, addRandomizeColumn(plan, randomize_seed.value()));
   }

   ARROW_ASSIGN_OR_RAISE(plan.top_node, addSortNode(plan, getOutputSchema(), ordering));

   if (randomize_seed.has_value()) {
      ARROW_ASSIGN_OR_RAISE(plan.top_node, removeRandomizeColumn(plan));
   }

   return plan;
}

}  // namespace silo::query_engine::operators
