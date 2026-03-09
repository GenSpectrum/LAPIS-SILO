#include "zstd_decompress_node.h"

#include <chrono>
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <arrow/compute/expression.h>
#include <spdlog/spdlog.h>

#include "evobench/evobench.hpp"
#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"
#include "silo/common/size_constants.h"
#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/query_engine/exec_node/throttled_batch_reslicer.h"
#include "silo/query_engine/exec_node/zstd_decompress_expression.h"
#include "silo/query_engine/operators/compute_partition_filters.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/column_type_visitor.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/column/zstd_compressed_string_column.h"
#include "silo/storage/table.h"

namespace silo::query_engine::operators {

namespace {

using silo::schema::ColumnIdentifier;
using silo::schema::TableSchema;
using silo::storage::column::SequenceColumnPartition;
using silo::storage::column::ZstdCompressedStringColumnPartition;

class ColumnToReferenceSequenceVisitor {
  public:
   template <silo::storage::column::Column ColumnType>
   std::optional<std::string> operator()(
      const TableSchema& /*table_schema*/,
      const ColumnIdentifier& /*column_identifier*/
   ) {
      return std::nullopt;
   }
};

template <>
std::optional<std::string> ColumnToReferenceSequenceVisitor::operator(
)<SequenceColumnPartition<silo::Nucleotide>>(
   const TableSchema& table_schema,
   const ColumnIdentifier& column_identifier
) {
   auto* metadata =
      table_schema
         .getColumnMetadata<SequenceColumnPartition<silo::Nucleotide>>(column_identifier.name)
         .value();
   std::string reference;
   std::ranges::transform(
      metadata->reference_sequence, std::back_inserter(reference), silo::Nucleotide::symbolToChar
   );
   return reference;
}

template <>
std::optional<std::string> ColumnToReferenceSequenceVisitor::operator(
)<SequenceColumnPartition<silo::AminoAcid>>(
   const TableSchema& table_schema,
   const ColumnIdentifier& column_identifier
) {
   auto* metadata =
      table_schema
         .getColumnMetadata<SequenceColumnPartition<silo::AminoAcid>>(column_identifier.name)
         .value();
   std::string reference;
   std::ranges::transform(
      metadata->reference_sequence, std::back_inserter(reference), silo::AminoAcid::symbolToChar
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

std::vector<schema::ColumnIdentifier> ZstdDecompressNode::getOutputSchema() const {
   std::vector<schema::ColumnIdentifier> output_schema;
   for (const auto& input_field : child->getOutputSchema()) {
      if (isSequenceColumn(input_field.type)) {
         output_schema.emplace_back(input_field.name, schema::ColumnType::STRING);
      } else {
         output_schema.push_back(input_field);
      }
   }
   return output_schema;
}

arrow::Result<PartialArrowPlan> ZstdDecompressNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& tables,
   const config::QueryOptions& query_options
) const {
   ARROW_ASSIGN_OR_RAISE(auto plan, child->toQueryPlan(tables, query_options));
   const auto& input_ordering = plan.top_node->ordering();

   size_t sum_of_reference_genome_sizes = 0;

   std::vector<arrow::compute::Expression> column_expressions;
   std::vector<std::string> column_names;
   for (const auto& column : child->getOutputSchema()) {
      if (auto iter = table_schemas_for_decompression.find(column);
          iter != table_schemas_for_decompression.end()) {
         auto reference = silo::storage::column::visit(
            column.type, ColumnToReferenceSequenceVisitor{}, *iter->second, column
         );
         column_expressions.push_back(silo::query_engine::exec_node::ZstdDecompressExpression::make(
            arrow::compute::field_ref(arrow::FieldRef{column.name}), reference.value()
         ));
         sum_of_reference_genome_sizes += reference.value().length();
      } else {
         column_expressions.push_back(arrow::compute::field_ref(arrow::FieldRef{column.name}));
      }
      column_names.push_back(column.name);
   }

   // Add a sink and source to let arrow apply correct backpressure
   arrow::AsyncGenerator<std::optional<arrow::ExecBatch>> batch_generator;
   arrow::acero::BackpressureMonitor* backpressure_monitor;
   std::shared_ptr<arrow::Schema> schema_of_sequence_batches;
   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode(
         "sink",
         plan.plan.get(),
         {plan.top_node},
         arrow::acero::SinkNodeOptions{
            &batch_generator,
            &schema_of_sequence_batches,
            arrow::acero::BackpressureOptions{
               /*.resume_if_below =*/silo::common::S_16_KB,
               /*.pause_if_above =*/silo::common::S_64_MB
            },
            &backpressure_monitor
         }
      )
   );
   plan.top_node->SetLabel(
      "additional sink node to help backpressure application before zstd decompression"
   );

   SILO_ASSERT_GT(sum_of_reference_genome_sizes, 0U);

   auto maximum_batch_size =
      static_cast<int64_t>(std::max(silo::common::S_64_MB / sum_of_reference_genome_sizes, 1UL));

   constexpr std::chrono::milliseconds TARGET_BATCH_RATE{667};

   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode(
         "source",
         plan.plan.get(),
         {},
         arrow::acero::SourceNodeOptions{
            schema_of_sequence_batches,
            silo::query_engine::exec_node::ThrottledBatchReslicer{
               batch_generator, maximum_batch_size, TARGET_BATCH_RATE, backpressure_monitor
            },
            input_ordering
         }
      )
   );
   plan.top_node->SetLabel(
      "additional source node to help backpressure application before zstd decompression"
   );

   const arrow::acero::ProjectNodeOptions project_options{column_expressions, column_names};
   ARROW_ASSIGN_OR_RAISE(
      plan.top_node,
      arrow::acero::MakeExecNode(
         std::string{"project"}, plan.plan.get(), {plan.top_node}, project_options
      )
   );

   return plan;
}

}  // namespace silo::query_engine::operators
