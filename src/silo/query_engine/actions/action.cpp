#include "silo/query_engine/actions/action.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <memory>
#include <utility>

#include <arrow/acero/exec_plan.h>
#include <arrow/builder.h>
#include <arrow/compute/ordering.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "silo/common/size_constants.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/throttled_batch_reslicer.h"
#include "silo/query_engine/exec_node/zstd_decompress_expression.h"
#include "silo/query_engine/illegal_query_exception.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::query_engine::actions {

Action::Action() = default;

void Action::setOrdering(
   const std::vector<OrderByField>& order_by_fields_,
   std::optional<uint32_t> limit_,
   std::optional<uint32_t> offset_,
   std::optional<uint32_t> randomize_seed_
) {
   order_by_fields = order_by_fields_;
   limit = limit_;
   offset = offset_;
   randomize_seed = randomize_seed_;
}

void Action::setOrderByFields(const std::vector<OrderByField>& order_by_fields_) {
   order_by_fields = order_by_fields_;
}

using arrow::compute::NullPlacement;

const std::string RANDOMIZE_HASH_FIELD_NAME{"__SILO_RANDOMIZE_HASH"};

std::optional<arrow::Ordering> Action::getOrdering() const {
   using arrow::compute::SortOrder;

   std::vector<arrow::compute::SortKey> sort_keys;
   for (const auto& order_by_field : order_by_fields) {
      auto sort_order = order_by_field.ascending ? SortOrder::Ascending : SortOrder::Descending;
      sort_keys.emplace_back(order_by_field.name, sort_order);
   }

   if (randomize_seed.has_value()) {
      sort_keys.emplace_back(RANDOMIZE_HASH_FIELD_NAME);
   }

   if (sort_keys.empty()) {
      return std::nullopt;
   }

   auto first_sort_key = sort_keys.at(0);
   const auto null_placement = first_sort_key.order == arrow::compute::SortOrder::Ascending
                                  ? NullPlacement::AtStart
                                  : NullPlacement::AtEnd;

   return arrow::Ordering{sort_keys, null_placement};
}

std::vector<std::string> Action::deduplicateOrderPreserving(const std::vector<std::string>& fields
) {
   std::vector<std::string> unique_fields;
   std::unordered_set<std::string> seen;

   for (const auto& field : fields) {
      if (seen.insert(field).second) {
         unique_fields.push_back(field);
      }
   }
   return unique_fields;
}

QueryPlan Action::toQueryPlan(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> partition_filters,
   const config::QueryOptions& query_options,
   std::string_view request_id
) {
   validateOrderByFields(table->schema);
   auto query_plan =
      toQueryPlanImpl(std::move(table), std::move(partition_filters), query_options, request_id);
   if (!query_plan.status().ok()) {
      SILO_PANIC("Arrow error: {}", query_plan.status().ToString());
   };
   return query_plan.ValueUnsafe();
}

arrow::Result<arrow::acero::ExecNode*> Action::addSortNode(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node,
   const std::vector<schema::ColumnIdentifier>& output_fields,
   const arrow::Ordering& ordering,
   std::optional<size_t> /*num_rows_to_produce*/
) {
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> generator;
   // TODO(#800) add optimized sorting when limit is supplied
   //      if (limit.has_value()) {
   //         auto number_of_rows_to_be_sorted = limit.value() + offset.value_or(0);
   //         ARROW_ASSIGN_OR_RAISE(
   //            node,
   //            arrow::acero::MakeExecNode(
   //               "select_k_sink",
   //               arrow_plan,
   //               {node},
   //               arrow::acero::SelectKSinkNodeOptions{
   //                  arrow::SelectKOptions(number_of_rows_to_be_sorted,
   //                  ordering.value().sort_keys()), &generator
   //               }
   //            )
   //         );
   //      } else {
   ARROW_ASSIGN_OR_RAISE(
      node,
      arrow::acero::MakeExecNode(
         "order_by_sink",
         arrow_plan,
         {node},
         arrow::acero::OrderBySinkNodeOptions{arrow::SortOptions{ordering}, &generator}
      )
   );
   node->SetLabel("order by");
   //      }
   auto schema = exec_node::columnsToArrowSchema(output_fields);
   return arrow::acero::MakeExecNode(
      "source",
      arrow_plan,
      {},
      arrow::acero::SourceNodeOptions{schema, std::move(generator), ordering}
   );
}

namespace {

uint64_t hash64(uint64_t value, uint64_t seed) {
   value ^= seed;
   value ^= value >> 33;
   value *= 0xff51afd7ed558ccdULL;
   value ^= value >> 33;
   value *= 0xc4ceb9fe1a85ec53ULL;
   value ^= value >> 33;
   return value;
}

arrow::Result<arrow::acero::ExecNode*> removeRandomizeColumn(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node
) {
   std::vector<arrow::Expression> field_refs;
   for (const auto& field : node->output_schema()->fields()) {
      if (field->name() != RANDOMIZE_HASH_FIELD_NAME) {
         field_refs.push_back(arrow::compute::field_ref(field->name()));
      }
   }
   auto options = arrow::acero::ProjectNodeOptions(field_refs);
   return arrow::acero::MakeExecNode("project", arrow_plan, {node}, options);
}

}  // namespace

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<arrow::acero::ExecNode*> Action::addRandomizeColumn(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node,
   size_t randomize_seed
) {
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> sequenced_batches;
   std::shared_ptr<arrow::Schema> schema_of_sequence_batches;
   ARROW_ASSIGN_OR_RAISE(
      node,
      arrow::acero::MakeExecNode(
         "sink",
         arrow_plan,
         {node},
         arrow::acero::SinkNodeOptions{&sequenced_batches, &schema_of_sequence_batches}
      )
   );
   node->SetLabel("input to randomize column projection");
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
               const uint64_t hash = hash64(start_of_batch + i, randomize_seed);
               ARROW_RETURN_NOT_OK(randomize_column_builder.Append(hash));
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
      node,
      arrow::acero::MakeExecNode(
         "source",
         arrow_plan,
         {},
         arrow::acero::SourceNodeOptions{output_schema, std::move(sequenced_batches_with_hash_id)}
      )
   );
   node->SetLabel("output of randomize column projection");
   return node;
}

arrow::Result<arrow::acero::ExecNode*> Action::addLimitAndOffsetNode(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node
) const {
   if (limit.has_value() || offset.has_value()) {
      CHECK_SILO_QUERY(
         !node->ordering().is_unordered(),
         "Offset and limit can only be applied if the output of the operation has some ordering. "
         "Implicit ordering such as in the case of Details/Fasta is also allowed, Aggregated "
         "however produces unordered results."
      );
      const arrow::acero::FetchNodeOptions fetch_options(
         offset.value_or(0), limit.value_or(UINT32_MAX)
      );
      return arrow::acero::MakeExecNode(
         std::string{arrow::acero::FetchNodeOptions::kName}, arrow_plan, {node}, fetch_options
      );
   }
   return node;
}

namespace {

using silo::schema::ColumnIdentifier;
using silo::schema::TableSchema;
using silo::storage::column::Column;
using silo::storage::column::SequenceColumnPartition;
using silo::storage::column::ZstdCompressedStringColumnPartition;

class ColumnToReferenceSequenceVisitor {
  public:
   template <Column ColumnType>
   std::optional<std::string> operator()(
      const TableSchema& /*table_schema*/,
      const ColumnIdentifier& /*column_identifier*/
   ) {
      return std::nullopt;
   }
};

template <>
std::optional<std::string> ColumnToReferenceSequenceVisitor::operator(
)<SequenceColumnPartition<Nucleotide>>(
   const TableSchema& table_schema,
   const ColumnIdentifier& column_identifier
) {
   auto* metadata =
      table_schema.getColumnMetadata<SequenceColumnPartition<Nucleotide>>(column_identifier.name)
         .value();
   std::string reference;
   std::ranges::transform(
      metadata->reference_sequence, std::back_inserter(reference), Nucleotide::symbolToChar
   );
   return reference;
}

template <>
std::optional<std::string> ColumnToReferenceSequenceVisitor::operator(
)<SequenceColumnPartition<AminoAcid>>(
   const TableSchema& table_schema,
   const ColumnIdentifier& column_identifier
) {
   auto* metadata =
      table_schema.getColumnMetadata<SequenceColumnPartition<AminoAcid>>(column_identifier.name)
         .value();
   std::string reference;
   std::ranges::transform(
      metadata->reference_sequence, std::back_inserter(reference), AminoAcid::symbolToChar
   );
   return reference;
}

template <>
std::optional<std::string> ColumnToReferenceSequenceVisitor::operator(
)<ZstdCompressedStringColumnPartition>(
   const TableSchema& table_schema,
   const ColumnIdentifier& column_identifier
) {
   auto* metadata =
      table_schema.getColumnMetadata<ZstdCompressedStringColumnPartition>(column_identifier.name)
         .value();
   return metadata->dictionary_string;
}

}  // namespace

arrow::Result<arrow::acero::ExecNode*> Action::addZstdDecompressNode(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node,
   const silo::schema::TableSchema& table_schema
) const {
   auto output_fields = getOutputSchema(table_schema);
   const bool needs_decompression =
      std::ranges::any_of(output_fields, [](const auto& column_identifier) {
         return schema::isSequenceColumn(column_identifier.type);
      });
   if (needs_decompression) {
      size_t sum_of_reference_genome_sizes = 0;

      std::vector<arrow::compute::Expression> column_expressions;
      std::vector<std::string> column_names;
      for (const auto& column : getOutputSchema(table_schema)) {
         if (auto reference = storage::column::visit(
                column.type, ColumnToReferenceSequenceVisitor{}, table_schema, column
             )) {
            column_expressions.push_back(exec_node::ZstdDecompressExpression::make(
               arrow::compute::field_ref(arrow::FieldRef{column.name}), reference.value()
            ));
            sum_of_reference_genome_sizes += reference.value().length();
         } else {
            column_expressions.push_back(arrow::compute::field_ref(arrow::FieldRef{column.name}));
         }
         column_names.push_back(column.name);
      }

      // We add a sink and source to let arrow apply correct backpressure
      arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> batch_generator;
      arrow::acero::BackpressureMonitor* backpressure_monitor;
      std::shared_ptr<arrow::Schema> schema_of_sequence_batches;
      ARROW_ASSIGN_OR_RAISE(
         node,
         arrow::acero::MakeExecNode(
            "sink",
            arrow_plan,
            {node},
            arrow::acero::SinkNodeOptions{
               &batch_generator,
               &schema_of_sequence_batches,
               arrow::acero::BackpressureOptions{
                  /*.resume_if_below =*/common::S_16_KB,  // 16 KB
                  /*.pause_if_above =*/common::S_64_MB    // 64 MB
               },
               &backpressure_monitor
            }
         )
      );
      node->SetLabel(
         "additional sink node to help backpressure application before zstd decompression"
      );

      SILO_ASSERT_GT(sum_of_reference_genome_sizes, 0U);

      // We aim for 64 MB batch size to give the plan time to apply backpressure
      auto maximum_batch_size =
         static_cast<int64_t>(std::max(common::S_64_MB / sum_of_reference_genome_sizes, 1UL));

      // Delay delivery of large number of batches to about 100 MB per second
      // A batch targets 64 MB, therefore we allow 1.5 batches per second.
      // Therefore, we should never emit more than one resliced batch per 0.667 seconds
      constexpr std::chrono::milliseconds TARGET_BATCH_RATE{667};

      ARROW_ASSIGN_OR_RAISE(
         node,
         arrow::acero::MakeExecNode(
            "source",
            arrow_plan,
            {},
            arrow::acero::SourceNodeOptions{
               schema_of_sequence_batches,
               exec_node::ThrottledBatchReslicer{
                  batch_generator, maximum_batch_size, TARGET_BATCH_RATE, backpressure_monitor
               }
            }
         )
      );
      node->SetLabel(
         "additional source node to help backpressure application before zstd decompression"
      );

      const arrow::acero::ProjectNodeOptions project_options{column_expressions, column_names};
      return arrow::acero::MakeExecNode(
         std::string{"project"}, arrow_plan, {node}, project_options
      );
   }
   return node;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<arrow::acero::ExecNode*> Action::addOrderingNodes(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node,
   const silo::schema::TableSchema& table_schema
) const {
   if (auto ordering = getOrdering()) {
      std::optional<uint32_t> num_rows_to_produce;
      if (limit.has_value()) {
         num_rows_to_produce = limit.value() + offset.value_or(0);
      }

      auto randomize = randomize_seed;
      if (randomize) {
         ARROW_ASSIGN_OR_RAISE(node, addRandomizeColumn(arrow_plan, node, randomize_seed.value()));
      }
      ARROW_ASSIGN_OR_RAISE(
         node,
         addSortNode(
            arrow_plan, node, getOutputSchema(table_schema), ordering.value(), num_rows_to_produce
         )
      );
      if (randomize) {
         ARROW_ASSIGN_OR_RAISE(node, removeRandomizeColumn(arrow_plan, node));
      }
   }
   return node;
}

}  // namespace silo::query_engine::actions
