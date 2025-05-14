#include "silo/query_engine/exec_node/table_scan.h"

namespace silo::query_engine::exec_node {

namespace {

template <typename SymbolType>
arrow::Status appendSequences(
   const storage::column::SequenceColumnPartition<SymbolType>& sequence_store,
   const roaring::Roaring& row_ids,
   arrow::StringBuilder& output_array
) {
   std::string partition_reference;
   std::ranges::transform(
      sequence_store.metadata->reference_sequence,
      std::back_inserter(partition_reference),
      SymbolType::symbolToChar
   );

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
         for (size_t row_id : *position.getBitmap(symbol) & row_ids) {
            reconstructed_sequences[row_id][position_id] = SymbolType::symbolToChar(symbol);
         }
      }
   }

   for (size_t row_id : row_ids) {
      for (const size_t position_idx : sequence_store.missing_symbol_bitmaps.at(row_id)) {
         reconstructed_sequences[row_id][position_idx] =
            SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
      }
   }
   ARROW_RETURN_NOT_OK(output_array.AppendValues(reconstructed_sequences));
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
      table_scan_node.getColumnTypeArrayBuilders<storage::column::SequenceColumnPartition<Nucleotide>>()
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
      table_scan_node.getColumnTypeArrayBuilders<storage::column::SequenceColumnPartition<AminoAcid>>()
         .at(column_name);
   return appendSequences<AminoAcid>(
      table_partition.columns.aa_columns.at(column_name), row_ids, *array
   );
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
      using type = ArrowBuilderSelector<Column>::value_type;
      auto value = table_partition.columns.getValue(column_name, row_id);
      if (!value.has_value()) {
         SILO_PANIC("Called getValue on column {} that does not exist", column_name);
      }
      ARROW_RETURN_NOT_OK(array->Append(get<type>(value.value())));
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
   for (size_t partition_idx = 0; partition_idx < table->getNumberOfPartitions(); ++partition_idx) {
      auto& filter_for_partition = partition_filters.at(partition_idx);
      if (filter_for_partition->isEmpty()) {
         continue;
      }
      size_t num_rows_produced_in_part = 0;
      uint32_t start_of_next_batch;  // Row id in SILO
      uint32_t end_of_next_batch;    // Row id in SILO
      while (true) {
         if (!filter_for_partition->select(num_rows_produced_in_part, &start_of_next_batch)) {
            break;
         }
         if (!filter_for_partition->select(
                num_rows_produced_in_part + MATERIALIZATION_CUTOFF - 1, &end_of_next_batch
             )) {
            filter_for_partition->select(
               filter_for_partition->cardinality() - 1, &end_of_next_batch
            );
            num_rows_produced_in_part = filter_for_partition->cardinality();
         } else {
            num_rows_produced_in_part += MATERIALIZATION_CUTOFF;
         };
         auto row_ids = roaring::Roaring{};
         row_ids.addRange(start_of_next_batch, end_of_next_batch + 1);
         row_ids &= *filter_for_partition;
         ARROW_RETURN_NOT_OK(appendEntries(table->getPartition(partition_idx), row_ids));
         ARROW_RETURN_NOT_OK(flushOutput());
      }
   }
   ARROW_RETURN_NOT_OK(flushOutput());
   ARROW_RETURN_NOT_OK(output_->InputFinished(this, num_batches));
   return arrow::Status::OK();
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
      storage::column::visit(field.type, [&]<storage::column::Column Column>() {
         auto array = getColumnTypeArrayBuilders<Column>().at(field.name);
         data.push_back(array->Finish().ValueOrDie());
      });
   }
   arrow::ExecBatch exec_batch;
   ARROW_ASSIGN_OR_RAISE(exec_batch, arrow::compute::ExecBatch::Make(data));
   ARROW_RETURN_NOT_OK(this->output_->InputReceived(static_cast<ExecNode*>(this), exec_batch));
   ++num_batches;
   return arrow::Status::OK();
}

}  // namespace silo::query_engine::exec_node
