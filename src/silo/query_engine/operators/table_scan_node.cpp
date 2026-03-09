#include "silo/query_engine/operators/table_scan_node.h"

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

arrow::Result<arrow::acero::ExecNode*> addZstdDecompressNode(
   arrow::acero::ExecPlan* arrow_plan,
   arrow::acero::ExecNode* node,
   const silo::schema::TableSchema& table_schema,
   const std::vector<silo::schema::ColumnIdentifier>& output_fields
) {
   const auto& input_ordering = node->ordering();

   const bool needs_decompression =
      std::ranges::any_of(output_fields, [](const auto& column_identifier) {
         return silo::schema::isSequenceColumn(column_identifier.type);
      });
   if (!needs_decompression) {
      return node;
   }

   size_t sum_of_reference_genome_sizes = 0;

   std::vector<arrow::compute::Expression> column_expressions;
   std::vector<std::string> column_names;
   for (const auto& column : output_fields) {
      if (auto reference = silo::storage::column::visit(
             column.type, ColumnToReferenceSequenceVisitor{}, table_schema, column
          )) {
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
      node,
      arrow::acero::MakeExecNode(
         "sink",
         arrow_plan,
         {node},
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
   node->SetLabel("additional sink node to help backpressure application before zstd decompression"
   );

   SILO_ASSERT_GT(sum_of_reference_genome_sizes, 0U);

   auto maximum_batch_size =
      static_cast<int64_t>(std::max(silo::common::S_64_MB / sum_of_reference_genome_sizes, 1UL));

   constexpr std::chrono::milliseconds TARGET_BATCH_RATE{667};

   ARROW_ASSIGN_OR_RAISE(
      node,
      arrow::acero::MakeExecNode(
         "source",
         arrow_plan,
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
   node->SetLabel(
      "additional source node to help backpressure application before zstd decompression"
   );

   const arrow::acero::ProjectNodeOptions project_options{column_expressions, column_names};
   return arrow::acero::MakeExecNode(std::string{"project"}, arrow_plan, {node}, project_options);
}

}  // namespace

namespace silo::query_engine::operators {

TableScanNode::TableScanNode(
   std::shared_ptr<storage::Table> table,
   std::unique_ptr<filter::expressions::Expression> filter,
   std::vector<schema::ColumnIdentifier> fields
)
    : table(std::move(table)),
      filter(std::move(filter)),
      fields(std::move(fields)) {}

std::vector<schema::ColumnIdentifier> TableScanNode::getOutputSchema() const {
   return fields;
}

// TODO.TAE needed?
std::string_view TableScanNode::getType() const {
   return "TableScanNode";
}

arrow::Result<PartialArrowPlan> TableScanNode::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& query_options
) const {
   auto partition_filters = computePartitionFilters(filter, *table);

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   arrow::acero::ExecNode* node;
   ARROW_ASSIGN_OR_RAISE(
      node,
      exec_node::makeTableScan(
         arrow_plan.get(),
         fields,
         std::move(partition_filters),
         table,
         query_options.materialization_cutoff
      )
   );

   // TODO.TAE this is too early -> after sorting!!
   ARROW_ASSIGN_OR_RAISE(
      node, addZstdDecompressNode(arrow_plan.get(), node, table->schema, fields)
   );

   return PartialArrowPlan{node, arrow_plan};
}

}  // namespace silo::query_engine::operators
