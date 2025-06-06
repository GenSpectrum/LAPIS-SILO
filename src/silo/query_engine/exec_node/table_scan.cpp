#include "silo/query_engine/exec_node/table_scan.h"
#include "silo/query_engine/batched_bitmap_reader.h"

namespace silo::query_engine::exec_node {

namespace {

template <typename SymbolType>
arrow::Status appendSequences(
   const storage::column::SequenceColumnPartition<SymbolType>& sequence_store,
   const roaring::Roaring& row_ids,
   arrow::BinaryBuilder& output_array
) {
   std::string general_reference;
   std::ranges::transform(
      sequence_store.metadata->reference_sequence,
      std::back_inserter(general_reference),
      SymbolType::symbolToChar
   );

   std::string partition_reference = general_reference;
   for (const auto& [position_id, symbol] :
        sequence_store.indexing_differences_to_reference_sequence) {
      partition_reference[position_id] = SymbolType::symbolToChar(symbol);
   }

   std::vector<std::string> reconstructed_sequences(row_ids.cardinality(), partition_reference);

   for (size_t position_id = 0; position_id < sequence_store.positions.size(); position_id++) {
      const Position<SymbolType>& position = sequence_store.positions.at(position_id);
      for (const auto symbol : SymbolType::SYMBOLS) {
         if (position.isSymbolFlipped(symbol) || position.isSymbolDeleted(symbol)) {
            continue;
         }
         roaring::Roaring current_row_ids_with_that_mutation =
            row_ids & *position.getBitmap(symbol);

         std::vector<uint32_t> current_row_ids_with_that_mutation_as_vector(
            current_row_ids_with_that_mutation.cardinality()
         );
         current_row_ids_with_that_mutation.toUint32Array(
            current_row_ids_with_that_mutation_as_vector.data()
         );

         std::vector<uint64_t> ids_in_reconstructed_sequences(
            current_row_ids_with_that_mutation_as_vector.size()
         );
         row_ids.rank_many(
            current_row_ids_with_that_mutation_as_vector.begin().base(),
            current_row_ids_with_that_mutation_as_vector.end().base(),
            ids_in_reconstructed_sequences.data()
         );

         for (auto id_in_reconstructed_sequences : ids_in_reconstructed_sequences) {
            // Ranks are 1-indexed
            reconstructed_sequences.at(id_in_reconstructed_sequences - 1).at(position_id) =
               SymbolType::symbolToChar(symbol);
         }
      }
   }

   size_t id_in_reconstructed_sequences = 0;
   for (size_t row_id : row_ids) {
      for (const size_t position_idx : sequence_store.missing_symbol_bitmaps.at(row_id)) {
         reconstructed_sequences.at(id_in_reconstructed_sequences).at(position_idx) =
            SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
      }
      id_in_reconstructed_sequences++;
   }
   ARROW_RETURN_NOT_OK(output_array.Reserve(reconstructed_sequences.size()));
   auto reference_sequence = general_reference;
   auto dictionary = std::make_shared<silo::ZstdCDictionary>(reference_sequence, 3);
   silo::ZstdCompressor compressor{dictionary};
   for (const auto& reconstructed_sequence : reconstructed_sequences) {
      ARROW_RETURN_NOT_OK(output_array.Append(compressor.compress(reconstructed_sequence)));
   }
   return arrow::Status::OK();
}

class ColumnEntryAppender {
  public:
   template <storage::column::Column Column>
   arrow::Status operator()(
      TableScan& table_scan_node,
      const std::string& column_name,
      const storage::TablePartition& table_partition,
      const roaring::Roaring& row_ids
   );
};

template <>
arrow::Status ColumnEntryAppender::operator()<storage::column::SequenceColumnPartition<Nucleotide>>(
   TableScan& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   auto array =
      table_scan_node
         .getColumnTypeArrayBuilders<storage::column::SequenceColumnPartition<Nucleotide>>()
         .at(column_name);
   return appendSequences<Nucleotide>(
      table_partition.columns.nuc_columns.at(column_name), row_ids, *array
   );
}

template <>
arrow::Status ColumnEntryAppender::operator()<storage::column::SequenceColumnPartition<AminoAcid>>(
   TableScan& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   auto array =
      table_scan_node
         .getColumnTypeArrayBuilders<storage::column::SequenceColumnPartition<AminoAcid>>()
         .at(column_name);
   return appendSequences<AminoAcid>(
      table_partition.columns.aa_columns.at(column_name), row_ids, *array
   );
}

template <>
arrow::Status ColumnEntryAppender::operator()<storage::column::ZstdCompressedStringColumnPartition>(
   TableScan& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   auto array =
      table_scan_node
         .getColumnTypeArrayBuilders<storage::column::ZstdCompressedStringColumnPartition>()
         .at(column_name);
   auto column =
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
   TableScan& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   auto array = table_scan_node.getColumnTypeArrayBuilders<Column>().at(column_name);
   for (auto row_id : row_ids) {
      using type = typename ArrowBuilderSelector<Column>::value_type;
      auto value = table_partition.columns.getValue(column_name, row_id);
      if (value.has_value()) {
         ARROW_RETURN_NOT_OK(array->Append(get<type>(value.value())));
      } else {
         ARROW_RETURN_NOT_OK(array->AppendNull());
      }
   }
   return arrow::Status::OK();
}

}  // namespace

arrow::Status TableScan::appendEntries(
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   for (const auto& field : output_fields) {
      ARROW_RETURN_NOT_OK(storage::column::visit(
         field.type, ColumnEntryAppender{}, *this, field.name, table_partition, row_ids
      ));
   }
   return arrow::Status::OK();
}

arrow::Status TableScan::produce() {
   SPDLOG_TRACE("TableScan::produce");
   for (size_t partition_idx = 0; partition_idx < table->getNumberOfPartitions(); ++partition_idx) {
      auto& filter_for_partition = partition_filters.at(partition_idx);
      silo::query_engine::BatchedBitmapReader reader{filter_for_partition, batch_size_cutoff - 1};
      while (auto row_ids = reader.nextBatch()) {
         ARROW_RETURN_NOT_OK(appendEntries(table->getPartition(partition_idx), row_ids.value()));
         ARROW_RETURN_NOT_OK(flushOutput());
      }
   }
   SPDLOG_TRACE("Calling InputFinished on _output:");
   return output_->InputFinished(this, num_batches_produced);
}

void TableScan::prepareOutputArrays() {
   for (const auto& [name, type] : output_fields) {
      storage::column::visit(type, [&]<storage::column::Column Column>() {
         array_builders[type].emplace(name, std::make_unique<ArrowBuilder<Column>>());
      });
   }
}

arrow::Status TableScan::flushOutput() {
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
   arrow::ExecBatch exec_batch;
   ARROW_ASSIGN_OR_RAISE(exec_batch, arrow::compute::ExecBatch::Make(data));
   exec_batch.index = num_batches_produced++;
   ARROW_RETURN_NOT_OK(this->output_->InputReceived(static_cast<ExecNode*>(this), exec_batch));
   return arrow::Status::OK();
}

}  // namespace silo::query_engine::exec_node
