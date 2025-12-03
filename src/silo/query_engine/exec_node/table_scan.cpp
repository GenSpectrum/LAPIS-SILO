#include "silo/query_engine/exec_node/table_scan.h"
#include <utility>

#include <roaring/containers/array.h>
#include <roaring/containers/bitset.h>
#include <roaring/containers/containers.h>
#include <roaring/containers/run.h>
#include <roaring/roaring.h>
#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "silo/query_engine/batched_bitmap_reader.h"
#include "silo/storage/column/column_type_visitor.h"

namespace silo::query_engine::exec_node {

namespace {

template <typename SymbolType>
arrow::Status appendSequences(
   const storage::column::SequenceColumnPartition<SymbolType>& sequence_column_partition,
   const roaring::Roaring& row_ids,
   arrow::BinaryBuilder& output_array
) {
   size_t cardinality = row_ids.cardinality();

   std::string general_reference = sequence_column_partition.reference_sequence_string;

   std::string partition_reference = general_reference;
   for (const auto& [position_id, symbol] :
        sequence_column_partition.indexing_differences_to_reference_sequence) {
      partition_reference[position_id] = SymbolType::symbolToChar(symbol);
   }

   std::vector<std::string> reconstructed_sequences;
   reconstructed_sequences.resize(cardinality, partition_reference);

   sequence_column_partition.vertical_sequence_index.overwriteSymbolsInSequences(
      reconstructed_sequences, row_ids
   );

   sequence_column_partition.horizontal_coverage_index
      .template overwriteCoverageInSequence<SymbolType>(reconstructed_sequences, row_ids);

   ARROW_RETURN_NOT_OK(output_array.Reserve(cardinality));
   auto reference_sequence = general_reference;
   auto dictionary = std::make_shared<silo::ZstdCDictionary>(reference_sequence, 3);
   silo::ZstdCompressor compressor{dictionary};
   for (const auto& reconstructed_sequence : reconstructed_sequences) {
      ARROW_RETURN_NOT_OK(output_array.Append(
         compressor.compress(reconstructed_sequence.data(), reconstructed_sequence.size())
      ));
   }
   return arrow::Status::OK();
}

class ColumnEntryAppender {
  public:
   template <storage::column::Column Column>
   arrow::Status operator()(
      ExecBatchBuilder& table_scan_node,
      const std::string& column_name,
      const storage::TablePartition& table_partition,
      const roaring::Roaring& row_ids
   );
};

template <>
arrow::Status ColumnEntryAppender::operator()<storage::column::SequenceColumnPartition<Nucleotide>>(
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   EVOBENCH_SCOPE(
      "ColumnEntryAppender",
      columnTypeToString(storage::column::SequenceColumnPartition<Nucleotide>::TYPE)
   );
   auto* array =
      table_scan_node
         .getColumnTypeArrayBuilders<storage::column::SequenceColumnPartition<Nucleotide>>()
         .at(column_name);
   return appendSequences<Nucleotide>(
      table_partition.columns.nuc_columns.at(column_name), row_ids, *array
   );
}

template <>
arrow::Status ColumnEntryAppender::operator()<storage::column::SequenceColumnPartition<AminoAcid>>(
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   EVOBENCH_SCOPE(
      "ColumnEntryAppender",
      columnTypeToString(storage::column::SequenceColumnPartition<AminoAcid>::TYPE)
   );
   auto* array =
      table_scan_node
         .getColumnTypeArrayBuilders<storage::column::SequenceColumnPartition<AminoAcid>>()
         .at(column_name);
   return appendSequences<AminoAcid>(
      table_partition.columns.aa_columns.at(column_name), row_ids, *array
   );
}

template <>
arrow::Status ColumnEntryAppender::operator()<storage::column::ZstdCompressedStringColumnPartition>(
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   EVOBENCH_SCOPE(
      "ColumnEntryAppender",
      columnTypeToString(storage::column::ZstdCompressedStringColumnPartition::TYPE)
   );
   auto* array =
      table_scan_node
         .getColumnTypeArrayBuilders<storage::column::ZstdCompressedStringColumnPartition>()
         .at(column_name);
   const auto& column =
      table_partition.columns.getColumns<storage::column::ZstdCompressedStringColumnPartition>().at(
         column_name
      );
   for (auto row_id : row_ids) {
      auto value = column.getCompressed(row_id);
      if (value.has_value()) {
         ARROW_RETURN_NOT_OK(array->Append(value.value()));
      } else {
         ARROW_RETURN_NOT_OK(array->AppendNull());
      }
   }
   return arrow::Status::OK();
}

template <storage::column::Column Column>
arrow::Status ColumnEntryAppender::operator()(
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   EVOBENCH_SCOPE("ColumnEntryAppender", columnTypeToString(Column::TYPE));
   auto array = table_scan_node.getColumnTypeArrayBuilders<Column>().at(column_name);
   for (auto row_id : row_ids) {
      auto& column = table_partition.columns.getColumns<Column>().at(column_name);
      if (column.isNull(row_id)) {
         ARROW_RETURN_NOT_OK(array->AppendNull());
      } else {
         if constexpr (std::is_same_v<Column, storage::column::StringColumnPartition>) {
            auto value = column.getValueString(row_id);
            ARROW_RETURN_NOT_OK(array->Append(value));
         } else if constexpr (std::
                                 is_same_v<Column, storage::column::IndexedStringColumnPartition>) {
            auto value = column.getValueString(row_id);
            ARROW_RETURN_NOT_OK(array->Append(value));
         } else if constexpr (std::is_same_v<Column, storage::column::DateColumnPartition>) {
            auto value = common::dateToString(column.getValue(row_id)).value();
            ARROW_RETURN_NOT_OK(array->Append(value));
         } else {
            auto value = column.getValue(row_id);
            ARROW_RETURN_NOT_OK(array->Append(value));
         }
      }
   }
   return arrow::Status::OK();
}

}  // namespace

ExecBatchBuilder::ExecBatchBuilder(std::vector<silo::schema::ColumnIdentifier> output_fields_)
    : output_fields(std::move(output_fields_)) {
   for (const auto& [name, type] : output_fields) {
      storage::column::visit(type, [&]<storage::column::Column Column>() {
         array_builders[type].emplace(name, std::make_shared<ArrowBuilder<Column>>());
      });
   }
}

arrow::Status ExecBatchBuilder::appendEntries(
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   EVOBENCH_SCOPE("ExecBatchBuilder", "appendEntries");
   for (const auto& field : output_fields) {
      ARROW_RETURN_NOT_OK(storage::column::visit(
         field.type, ColumnEntryAppender{}, *this, field.name, table_partition, row_ids
      ));
   }
   return arrow::Status::OK();
}

arrow::Result<arrow::ExecBatch> ExecBatchBuilder::finishBatch() {
   EVOBENCH_SCOPE("ExecBatchBuilder", "finishBatch");
   std::vector<arrow::Datum> data;
   for (auto& field : output_fields) {
      auto status = storage::column::visit(field.type, [&]<storage::column::Column Column>() {
         ARROW_ASSIGN_OR_RAISE(
            auto array, getColumnTypeArrayBuilders<Column>().at(field.name)->Finish()
         );
         data.push_back(array);
         return arrow::Status::OK();
      });
      ARROW_RETURN_NOT_OK(status);
   }
   return arrow::compute::ExecBatch::Make(data);
}

arrow::Result<std::optional<arrow::ExecBatch>> TableScanGenerator::produceNextBatch() {
   EVOBENCH_SCOPE("TableScanGenerator", "produceNextBatch");
   while (current_bitmap_reader.has_value()) {
      auto row_ids = current_bitmap_reader.value().nextBatch();
      if (row_ids.has_value()) {
         ARROW_RETURN_NOT_OK(exec_batch_builder.appendEntries(
            table->getPartition(current_partition_idx), row_ids.value()
         ));
         ARROW_ASSIGN_OR_RAISE(auto batch, exec_batch_builder.finishBatch());
         SPDLOG_DEBUG("Finished arrow::ExecBatch with length: {}", batch.length);
         return batch;
      }
      current_partition_idx++;
      if (current_partition_idx < partition_filters.size()) {
         current_bitmap_reader = BatchedBitmapReader{
            partition_filters.at(current_partition_idx).getConstReference(), batch_size_cutoff
         };
      } else {
         current_bitmap_reader = std::nullopt;
      }
   }
   return std::nullopt;
}

arrow::Result<arrow::acero::ExecNode*> makeTableScan(
   arrow::acero::ExecPlan* plan,
   const std::vector<silo::schema::ColumnIdentifier>& columns,
   std::vector<CopyOnWriteBitmap> partition_filters_,
   std::shared_ptr<const storage::Table> table,
   size_t batch_size_cutoff
) {
   exec_node::TableScanGenerator generator(
      columns, std::move(partition_filters_), std::move(table), batch_size_cutoff
   );
   arrow::acero::SourceNodeOptions source_node_options{
      exec_node::columnsToArrowSchema(columns), generator, arrow::Ordering::Implicit()
   };
   return arrow::acero::MakeExecNode("source", plan, {}, source_node_options);
}

}  // namespace silo::query_engine::exec_node
