#pragma once

#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/filter/expressions/expression.h"
#include "silo/query_engine/filter/operators/operator.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/legacy_result_producer.h"
#include "silo/query_engine/query_result.h"

namespace silo::query_engine::actions {

class FastaAligned : public Action {

   void validateOrderByFields(const schema::TableSchema& schema) const override;

   [[nodiscard]] QueryResult execute(
      const Database& database,
      std::vector<CopyOnWriteBitmap> bitmap_filter
   ) const override;

  public:
   std::vector<std::string> sequence_names;
   explicit FastaAligned(std::vector<std::string>&& sequence_names);

   arrow::Schema getOutputSchema(const silo::schema::TableSchema& table_schema) const override;
};


namespace {
template <typename SymbolType>
std::string reconstructSequence(
   const storage::column::SequenceColumnPartition<SymbolType>& sequence_store,
   uint32_t row_id
) {
   std::string reconstructed_sequence;
   std::ranges::transform(
      sequence_store.metadata->reference_sequence,
      std::back_inserter(reconstructed_sequence),
      SymbolType::symbolToChar
   );

   for (const auto& [position_id, symbol] :
        sequence_store.indexing_differences_to_reference_sequence) {
      reconstructed_sequence[position_id] = SymbolType::symbolToChar(symbol);
   }

   for (size_t position_id = 0; position_id < sequence_store.positions.size(); position_id++) {
      const Position<SymbolType>& position = sequence_store.positions.at(position_id);
      for (const auto symbol : SymbolType::SYMBOLS) {
         if (!position.isSymbolFlipped(symbol) && !position.isSymbolDeleted(symbol) &&
             position.getBitmap(symbol)->contains(row_id)) {
            reconstructed_sequence[position_id] = SymbolType::symbolToChar(symbol);
         }
      }
   }

   for (const size_t position_idx : sequence_store.missing_symbol_bitmaps.at(row_id)) {
      reconstructed_sequence[position_idx] = SymbolType::symbolToChar(SymbolType::SYMBOL_MISSING);
   }
   return reconstructed_sequence;
}
}

class FastaAlignedProducer : public arrow::acero::ExecNode {
   FastaAligned action;
  public:

   std::atomic<bool> running = true;
   std::thread producer_thread;

   std::string primary_key_column_name;
   arrow::StringBuilder primary_key_array;

   std::vector<arrow::StringBuilder> nuc_sequence_arrays;
   std::vector<arrow::StringBuilder> aa_sequence_arrays;

   std::vector<CopyOnWriteBitmap> partition_filters;
   const storage::Table& table;

  public:
   FastaAlignedProducer(arrow::acero::ExecPlan* plan, const FastaAligned& action, const filter::expressions::Expression& filter, const std::shared_ptr<Database> database)
       : arrow::acero::ExecNode(plan, {}, {}, std::make_shared<arrow::Schema>(action.getOutputSchema(database->schema.tables.at(schema::TableName::getDefault())))), action(action),
         table(database->table) {
      for (size_t partition_index = 0; partition_index != database->table.getNumberOfPartitions();
           partition_index++) {
         std::unique_ptr<Operator> part_filter = filter.compile(
            *database, database->table.getPartition(partition_index), Expression::AmbiguityMode::NONE
         );
         partition_filters.emplace_back(part_filter->evaluate());
      }
      prepareOutputArrays(database->schema.tables.at(schema::TableName::getDefault()), action.sequence_names);
      primary_key_column_name = database->schema.tables.at(schema::TableName::getDefault()).primary_key.name;
   }

   void prepareOutputArrays(
      const schema::TableSchema& schema,
      const std::vector<std::string>& sequence_names
   ) {
      for (const std::string& sequence_name : sequence_names) {
         auto column = schema.getColumn(sequence_name);
         CHECK_SILO_QUERY(
            column.has_value() && (column.value().type == schema::ColumnType::NUCLEOTIDE_SEQUENCE ||
                                   column.value().type == schema::ColumnType::AMINO_ACID_SEQUENCE),
            "Database does not contain a sequence with name: '" + sequence_name + "'"
         );
         // For now throw an error for aa sequences
         CHECK_SILO_QUERY(
            column.value().type == schema::ColumnType::NUCLEOTIDE_SEQUENCE,
            "For now only nuc sequences allowed!" // TODO remove check
         );
         if (column.value().type == schema::ColumnType::NUCLEOTIDE_SEQUENCE) {
            nuc_sequence_arrays.emplace_back();
         } else {
            aa_sequence_arrays.emplace_back();
         }
      }
   }

   virtual const char* kind_name() const override { return "LegacyResultProducer"; }

   virtual arrow::Status InputReceived(ExecNode* input, arrow::ExecBatch batch) override {
      SILO_PANIC("LegacyResultProducer does not support having inputs.");
   }

   virtual arrow::Status StopProducingImpl() override { SILO_UNIMPLEMENTED(); }

   /// Mark the inputs finished after the given number of batches.
   ///
   /// This may be called before all inputs are received.  This simply fixes
   /// the total number of incoming batches for an input, so that the ExecNode
   /// knows when it has received all input, regardless of order.
   virtual arrow::Status InputFinished(ExecNode* input, int total_batches) override {
      SILO_PANIC("LegacyResultProducer does not support having inputs.");
   }

   arrow::Status flushOutput() {
      std::vector<arrow::Datum> data;
      for(auto& array : nuc_sequence_arrays){
         data.push_back(array.Finish().ValueOrDie());
      }
      data.push_back(primary_key_array.Finish().ValueOrDie());
      arrow::ExecBatch exec_batch;
      ARROW_ASSIGN_OR_RAISE(exec_batch, arrow::compute::ExecBatch::Make(data));
      ARROW_RETURN_NOT_OK(this->output_->InputReceived(this, exec_batch));
      return arrow::Status::OK();
   }

   static constexpr size_t MATERIALIZATION_CUTOFF = 50000;

   arrow::Status produce() {
      if (running) {
         std::optional<QueryResultEntry> row;
         for(size_t partition_idx = 0; partition_idx < table.getNumberOfPartitions(); ++partition_idx){
            auto& filter_for_partition = partition_filters.at(partition_idx);
            if(filter_for_partition->isEmpty()){
               continue;
            }
            size_t num_rows_produced_in_part = 0;
            uint32_t start_of_next_batch; // Row id in SILO
            uint32_t end_of_next_batch;  // Row id in SILO
            while(true){
               if(!filter_for_partition->select(num_rows_produced_in_part, &start_of_next_batch)){
                  break;
               }
               if(!filter_for_partition->select(num_rows_produced_in_part + MATERIALIZATION_CUTOFF - 1, &end_of_next_batch)){
                  end_of_next_batch = filter_for_partition->select(filter_for_partition->cardinality() - 1, &end_of_next_batch);
                  num_rows_produced_in_part = filter_for_partition->cardinality();
               }
               else {
                  num_rows_produced_in_part += MATERIALIZATION_CUTOFF;
               };
               auto row_ids = roaring::Roaring{};
               row_ids.addRange(start_of_next_batch, end_of_next_batch + 1);
               row_ids &= *filter_for_partition;
               for(size_t row_id : row_ids){
                  ARROW_RETURN_NOT_OK(appendEntry(table.getPartition(partition_idx), row_id));
               }
               ARROW_RETURN_NOT_OK(flushOutput());
            }
         }
         ARROW_RETURN_NOT_OK(flushOutput());
      }
      return arrow::Status::OK();
   }

   arrow::Status appendEntry(
      const storage::TablePartition& table_partition,
      const uint32_t row_id
   ) {
      ARROW_RETURN_NOT_OK(primary_key_array.Append(get<std::string>(table_partition.columns.getValue(primary_key_column_name, row_id).value())));
      size_t sequence_idx = 0;
      for (auto& output_array : nuc_sequence_arrays) {
         const auto& sequence_store = table_partition.columns.nuc_columns.at(output_schema_->field(sequence_idx)->name());
         auto sequence = reconstructSequence<Nucleotide>(sequence_store, row_id);
         ARROW_RETURN_NOT_OK(output_array.Append(sequence));
         sequence_idx++;
      }
      for (auto& output_array : aa_sequence_arrays) {
         const auto& sequence_store = table_partition.columns.aa_columns.at(output_schema_->field(sequence_idx)->name());
         auto sequence = reconstructSequence<AminoAcid>(sequence_store, row_id);
         ARROW_RETURN_NOT_OK(output_array.Append(sequence));
         sequence_idx++;
      }
      return arrow::Status::OK();
   }

   arrow::Status StartProducing() override {
      running.store(true);
      producer_thread = std::thread([this]() {
         arrow::Status status = this->produce();
         if (!status.ok()) {
            // Handle error or propagate
            throw std::runtime_error("Err: " + status.ToString());
         }
      });
      return arrow::Status::OK();
   }

   arrow::Status StopProducing() override {
      if (producer_thread.joinable()) {
         producer_thread.join();
      }
      running.store(false);
      return arrow::Status::OK();
   }

   void PauseProducing(arrow::acero::ExecNode* output, int32_t counter) override {}

   void ResumeProducing(arrow::acero::ExecNode* output, int32_t counter) override {}
};

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<FastaAligned>& action);

}  // namespace silo::query_engine::actions
