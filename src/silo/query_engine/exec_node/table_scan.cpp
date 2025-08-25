#include "silo/query_engine/exec_node/table_scan.h"

#include <roaring/containers/array.h>
#include <roaring/containers/bitset.h>
#include <roaring/containers/containers.h>
#include <roaring/containers/run.h>
#include <roaring/roaring.h>
#include <roaring/roaring.hh>

#include "silo/query_engine/batched_bitmap_reader.h"

namespace silo::query_engine::exec_node {

namespace {

// Get the subset of A & B and compute their ranks with respect to A
// Example:    A     B     rank in A
//             3
//             4 --- 4 --> 2
//             5 --- 5 --> 3
//                   6
//             7
//             9 --- 9 --> 5
// TODO add tests
std::vector<uint64_t> roaringSubsetRanks(
   const roaring::internal::container_t* c_a,
   uint8_t type_a,
   const roaring::internal::container_t* c_b,
   uint8_t type_b,
   uint32_t base
) {
   uint8_t type_a_and_b;
   auto c_a_and_b = roaring::internal::container_and(c_a, type_a, c_b, type_b, &type_a_and_b);

   std::vector<uint32_t> a_and_b_as_vector;
   size_t required_cardinality =
      roaring::internal::container_get_cardinality(c_a_and_b, type_a_and_b);
   a_and_b_as_vector.resize(required_cardinality);

   roaring::internal::container_to_uint32_array(
      a_and_b_as_vector.data(), c_a_and_b, type_a_and_b, base
   );

   std::vector<uint64_t> ids_in_reconstructed_sequences(a_and_b_as_vector.size());
   roaring::internal::container_rank_many(
      c_a,
      type_a,
      base,
      a_and_b_as_vector.begin().base(),
      a_and_b_as_vector.end().base(),
      ids_in_reconstructed_sequences.data()
   );

   return ids_in_reconstructed_sequences;
}

template <typename SymbolType>
arrow::Status appendSequences(
   const storage::column::SequenceColumnPartition<SymbolType>& sequence_column_partition,
   const roaring::Roaring& row_ids,
   arrow::BinaryBuilder& output_array
) {
   std::string general_reference;
   std::ranges::transform(
      sequence_column_partition.metadata->reference_sequence,
      std::back_inserter(general_reference),
      SymbolType::symbolToChar
   );

   std::string partition_reference = general_reference;
   for (const auto& [position_id, symbol] :
        sequence_column_partition.indexing_differences_to_reference_sequence) {
      partition_reference[position_id] = SymbolType::symbolToChar(symbol);
   }

   size_t total_cardinality = 0;
   std::vector<std::vector<std::string>> reconstructed_sequences_by_roaring_container(
      row_ids.roaring.high_low_container.size
   );
   for (size_t idx = 0; idx < row_ids.roaring.high_low_container.size; ++idx) {
      auto cardinality = roaring::internal::container_get_cardinality(
         row_ids.roaring.high_low_container.containers[idx],
         row_ids.roaring.high_low_container.typecodes[idx]
      );
      total_cardinality += cardinality;
      reconstructed_sequences_by_roaring_container.at(idx).resize(cardinality, partition_reference);
   }

   for (const auto& [sequence_diff_key, sequence_diff] :
        sequence_column_partition.vertical_bitmaps) {
      if(sequence_diff_key.vertical_tile_index >= row_ids.roaring.high_low_container.size &&
          // There are no sequences to reconstruct here
          !reconstructed_sequences_by_roaring_container.at(sequence_diff_key.vertical_tile_index).empty()){
         continue;
      }

      auto v_index = sequence_diff_key.vertical_tile_index;

      auto ranks_in_reconstructed_sequences = roaringSubsetRanks(
         row_ids.roaring.high_low_container.containers[v_index],
         row_ids.roaring.high_low_container.typecodes[v_index],
         sequence_diff.container,
         sequence_diff.typecode,
         /*base=*/0  // Base rank is 0, because we have a reconstructed_sequences vector per
                     // container
      );

      auto& reconstructed_sequences =
         reconstructed_sequences_by_roaring_container.at(sequence_diff_key.vertical_tile_index);
      for (auto rank_in_reconstructed_sequences : ranks_in_reconstructed_sequences) {
         // Ranks are 1-indexed
         uint32_t id_in_reconstructed_sequences = rank_in_reconstructed_sequences - 1;
         reconstructed_sequences.at(id_in_reconstructed_sequences).at(sequence_diff_key.position) =
            SymbolType::symbolToChar(sequence_diff_key.symbol);
      }
   }

   for (size_t roaring_container_idx = 0;
        roaring_container_idx < row_ids.roaring.high_low_container.size;
        ++roaring_container_idx) {
      size_t id_in_reconstructed_sequences = 0;
      auto& reconstructed_sequences =
         reconstructed_sequences_by_roaring_container.at(roaring_container_idx);
      std::vector<uint32_t> current_row_ids_as_vector(reconstructed_sequences.size());
      roaring::internal::container_to_uint32_array(
         current_row_ids_as_vector.data(),
         row_ids.roaring.high_low_container.containers[roaring_container_idx],
         row_ids.roaring.high_low_container.typecodes[roaring_container_idx],
         row_ids.roaring.high_low_container.keys[roaring_container_idx]
      );
      for (size_t row_id : row_ids) {
         for (const size_t position_idx : sequence_column_partition.horizontal_bitmaps.at(row_id)) {
            reconstructed_sequences.at(id_in_reconstructed_sequences).at(position_idx) =
               SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
         }
         id_in_reconstructed_sequences++;
      }
   }

   ARROW_RETURN_NOT_OK(output_array.Reserve(total_cardinality));
   auto reference_sequence = general_reference;
   auto dictionary = std::make_shared<silo::ZstdCDictionary>(reference_sequence, 3);
   silo::ZstdCompressor compressor{dictionary};
   for (const auto& reconstructed_sequences_subvector :
        reconstructed_sequences_by_roaring_container) {
      for (const auto& reconstructed_sequence : reconstructed_sequences_subvector) {
         ARROW_RETURN_NOT_OK(output_array.Append(
            compressor.compress(reconstructed_sequence.data(), reconstructed_sequence.size())
         ));
      }
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
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   EVOBENCH_SCOPE(
      "ColumnEntryAppender",
      columnTypeToString(storage::column::SequenceColumnPartition<AminoAcid>::TYPE)
   );
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
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::TablePartition& table_partition,
   const roaring::Roaring& row_ids
) {
   EVOBENCH_SCOPE(
      "ColumnEntryAppender",
      columnTypeToString(storage::column::ZstdCompressedStringColumnPartition::TYPE)
   );
   auto array =
      table_scan_node
         .getColumnTypeArrayBuilders<storage::column::ZstdCompressedStringColumnPartition>()
         .at(column_name);
   auto& column =
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
   if (!current_bitmap_reader.has_value()) {
      return std::nullopt;
   }
   while (true) {
      auto row_ids = current_bitmap_reader.value().nextBatch();
      if (row_ids.has_value()) {
         ARROW_RETURN_NOT_OK(exec_batch_builder.appendEntries(
            table->getPartition(current_partition_idx), row_ids.value()
         ));
         return exec_batch_builder.finishBatch();
      }
      current_partition_idx++;
      if (current_partition_idx < partition_filters.size()) {
         current_bitmap_reader =
            BatchedBitmapReader{partition_filters.at(current_partition_idx), batch_size_cutoff};
      } else {
         current_bitmap_reader = std::nullopt;
      }
      return std::nullopt;
   }
}

arrow::Result<arrow::acero::ExecNode*> makeTableScan(
   arrow::acero::ExecPlan* plan,
   const std::vector<silo::schema::ColumnIdentifier>& columns,
   std::vector<CopyOnWriteBitmap> partition_filters_,
   std::shared_ptr<const storage::Table> table,
   size_t batch_size_cutoff
) {
   exec_node::TableScanGenerator generator(columns, partition_filters_, table, batch_size_cutoff);
   arrow::acero::SourceNodeOptions source_node_options{
      exec_node::columnsToArrowSchema(columns), generator, arrow::Ordering::Implicit()
   };
   return arrow::acero::MakeExecNode("source", plan, {}, source_node_options);
}

}  // namespace silo::query_engine::exec_node
