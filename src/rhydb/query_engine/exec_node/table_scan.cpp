#include "rhydb/query_engine/exec_node/table_scan.h"

#include <atomic>
#include <cstdint>
#include <exception>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include <arrow/acero/query_context.h>
#include <arrow/compute/ordering.h>
#include <arrow/util/async_util.h>
#include <arrow/util/future.h>

#include <roaring/containers/array.h>
#include <roaring/containers/bitset.h>
#include <roaring/containers/containers.h>
#include <roaring/containers/run.h>
#include <roaring/roaring.h>
#include <roaring/roaring.hh>

#include "evobench/evobench.hpp"
#include "rhydb/common/bitmap.h"
#include "rhydb/common/panic.h"
#include "rhydb/query_engine/batched_bitmap_reader.h"
#include "rhydb/query_engine/operators/compute_filter.h"
#include "rhydb/storage/column/column_type_visitor.h"

namespace rhydb::query_engine::exec_node {

using operators::CompiledFilter;

namespace {

template <typename SymbolType>
std::vector<std::string> reconstructNonNullSequences(
   const storage::column::SequenceColumn<SymbolType>& sequence_column,
   const Bitmap& non_null_row_ids
) {
   const std::string reference = sequence_column.local_reference_sequence_string;

   std::vector<std::string> reconstructed_sequences;
   reconstructed_sequences.resize(non_null_row_ids.cardinality(), reference);

   sequence_column.vertical_sequence_index.overwriteSymbolsInSequences(
      reconstructed_sequences, non_null_row_ids
   );

   sequence_column.horizontal_coverage_index.template overwriteCoverageInSequence<SymbolType>(
      reconstructed_sequences, non_null_row_ids
   );
   return reconstructed_sequences;
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Status appendSequences(
   const storage::column::SequenceColumn<SymbolType>& sequence_column,
   const Bitmap& row_ids,
   arrow::BinaryBuilder& output_array
) {
   const Bitmap non_null_row_ids = row_ids - Bitmap{&sequence_column.null_bitmap};
   auto reconstructed_non_null_sequences =
      reconstructNonNullSequences(sequence_column, non_null_row_ids);

   ARROW_RETURN_NOT_OK(output_array.Reserve(static_cast<int64_t>(row_ids.cardinality())));
   auto reference_sequence =
      SymbolType::sequenceToString(sequence_column.metadata->reference_sequence);
   auto dictionary = std::make_shared<rhydb::ZstdCDictionary>(reference_sequence, 3);
   rhydb::ZstdCompressor compressor{dictionary};

   auto reconstructed_sequence_iterator = reconstructed_non_null_sequences.begin();
   for (const auto& [chunk_key, view] : row_ids) {
      const uint32_t base = static_cast<uint32_t>(chunk_key) << 16U;
      for (const uint16_t low_bits : view) {
         if (sequence_column.isNull(storage::column::RowId::fromGlobal(base | low_bits))) {
            ARROW_RETURN_NOT_OK(output_array.AppendNull());
         } else {
            auto& reconstructed_sequence = *reconstructed_sequence_iterator;
            ARROW_RETURN_NOT_OK(output_array.Append(
               compressor.compress(reconstructed_sequence.data(), reconstructed_sequence.size())
            ));
            reconstructed_sequence_iterator++;
         }
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
      const storage::Table& table,
      const Bitmap& row_ids
   );
};

template <>
arrow::Status ColumnEntryAppender::operator()<storage::column::SequenceColumn<Nucleotide>>(
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::Table& table,
   const Bitmap& row_ids
) {
   EVOBENCH_SCOPE(
      "ColumnEntryAppender", columnTypeToString(storage::column::SequenceColumn<Nucleotide>::TYPE)
   );
   auto* array =
      table_scan_node.getColumnTypeArrayBuilders<storage::column::SequenceColumn<Nucleotide>>().at(
         column_name
      );
   return appendSequences<Nucleotide>(
      table.getColumn<storage::column::SequenceColumn<Nucleotide>>(column_name), row_ids, *array
   );
}

template <>
arrow::Status ColumnEntryAppender::operator()<storage::column::SequenceColumn<AminoAcid>>(
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::Table& table,
   const Bitmap& row_ids
) {
   EVOBENCH_SCOPE(
      "ColumnEntryAppender", columnTypeToString(storage::column::SequenceColumn<AminoAcid>::TYPE)
   );
   auto* array =
      table_scan_node.getColumnTypeArrayBuilders<storage::column::SequenceColumn<AminoAcid>>().at(
         column_name
      );
   return appendSequences<AminoAcid>(
      table.getColumn<storage::column::SequenceColumn<AminoAcid>>(column_name), row_ids, *array
   );
}

template <>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Status ColumnEntryAppender::operator()<storage::column::ZstdCompressedStringColumn>(
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::Table& table,
   const Bitmap& row_ids
) {
   EVOBENCH_SCOPE(
      "ColumnEntryAppender", columnTypeToString(storage::column::ZstdCompressedStringColumn::TYPE)
   );
   auto* array =
      table_scan_node.getColumnTypeArrayBuilders<storage::column::ZstdCompressedStringColumn>().at(
         column_name
      );
   const auto& column = table.getColumn<storage::column::ZstdCompressedStringColumn>(column_name);
   for (const auto& [chunk_key, view] : row_ids) {
      const uint32_t base = static_cast<uint32_t>(chunk_key) << 16U;
      for (const uint16_t low_bits : view) {
         auto value = column.getCompressed(storage::column::RowId::fromGlobal(base | low_bits));
         if (value.has_value()) {
            ARROW_RETURN_NOT_OK(array->Append(value.value()));
         } else {
            ARROW_RETURN_NOT_OK(array->AppendNull());
         }
      }
   }
   return arrow::Status::OK();
}

template <storage::column::Column Column>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Status ColumnEntryAppender::operator()(
   ExecBatchBuilder& table_scan_node,
   const std::string& column_name,
   const storage::Table& table,
   const Bitmap& row_ids
) {
   EVOBENCH_SCOPE("ColumnEntryAppender", columnTypeToString(Column::TYPE));
   auto& column = table.getColumn<Column>(column_name);
   auto array = table_scan_node.getColumnTypeArrayBuilders<Column>().at(column_name);
   for (const auto& [chunk_key, view] : row_ids) {
      const uint32_t base = static_cast<uint32_t>(chunk_key) << 16U;
      for (const uint16_t low_bits : view) {
         const auto row_id = storage::column::RowId::fromGlobal(base | low_bits);
         if (column.isNull(row_id)) {
            ARROW_RETURN_NOT_OK(array->AppendNull());
         } else {
            if constexpr (std::is_same_v<Column, storage::column::StringColumn>) {
               auto value = column.getValueString(row_id);
               ARROW_RETURN_NOT_OK(array->Append(value));
            } else if constexpr (std::is_same_v<Column, storage::column::DictionaryEncodedColumn>) {
               auto value = column.getValueString(row_id);
               ARROW_RETURN_NOT_OK(array->Append(value));
            } else {
               auto value = column.getValue(row_id);
               ARROW_RETURN_NOT_OK(array->Append(value));
            }
         }
      }
   }
   return arrow::Status::OK();
}

}  // namespace

ExecBatchBuilder::ExecBatchBuilder(std::vector<rhydb::schema::ColumnIdentifier> output_fields_)
    : output_fields(std::move(output_fields_)) {
   for (const auto& [name, type] : output_fields) {
      storage::column::visit(type, [&]<storage::column::Column Column> {
         array_builders[type].emplace(name, std::make_shared<ArrowBuilder<Column>>());
      });
   }
}

arrow::Status ExecBatchBuilder::appendEntries(const storage::Table& table, const Bitmap& row_ids) {
   EVOBENCH_SCOPE("ExecBatchBuilder", "appendEntries");
   for (const auto& field : output_fields) {
      ARROW_RETURN_NOT_OK(
         storage::column::visit(
            field.type, ColumnEntryAppender{}, *this, field.name, table, row_ids
         )
      );
   }
   return arrow::Status::OK();
}

arrow::Result<arrow::ExecBatch> ExecBatchBuilder::finishBatch(int64_t length) {
   EVOBENCH_SCOPE("ExecBatchBuilder", "finishBatch");
   std::vector<arrow::Datum> data;
   for (auto& field : output_fields) {
      auto status = storage::column::visit(field.type, [&]<storage::column::Column Column> {
         ARROW_ASSIGN_OR_RAISE(
            auto array, getColumnTypeArrayBuilders<Column>().at(field.name)->Finish()
         );
         data.push_back(array);
         return arrow::Status::OK();
      });
      ARROW_RETURN_NOT_OK(status);
   }
   // Pass the row count explicitly: a zero-column batch (e.g. a bare count(*) whose scan needs no
   // columns) has no arrays to infer the length from, and Arrow rejects ExecBatch::Make without it.
   return arrow::compute::ExecBatch::Make(std::move(data), length);
}

namespace {

class TableScan final : public arrow::acero::ExecNode {
  public:
   TableScan(
      arrow::acero::ExecPlan* plan,
      const std::vector<schema::ColumnIdentifier>& columns,
      CompiledFilter filter,
      std::shared_ptr<const storage::Table> table,
      size_t batch_size_cutoff
   )
       : ExecNode(plan, {}, {}, columnsToArrowSchema(columns)),
         exec_batch_builder_(columns),
         filter_(std::move(filter)),
         table_(std::move(table)),
         batch_size_cutoff_(batch_size_cutoff) {}

   [[nodiscard]] const char* kind_name() const override { return "TableScan"; }

   [[nodiscard]] const arrow::Ordering& ordering() const override {
      static const arrow::Ordering implicit = arrow::Ordering::Implicit();
      return implicit;
   }

   [[noreturn]] arrow::Status InputReceived(
      arrow::acero::ExecNode* /*input*/,
      arrow::ExecBatch /*batch*/
   ) override {
      RHYDB_UNREACHABLE();
   }

   [[noreturn]] arrow::Status
   InputFinished(arrow::acero::ExecNode* /*input*/, int /*total_batches*/) override {
      RHYDB_UNREACHABLE();
   }

   arrow::Status StartProducing() override {
      // Keeps the plan running while the scan is paused and no batch task is scheduled
      ARROW_ASSIGN_OR_RAISE(scan_done_, plan_->query_context()->BeginExternalTask(kind_name()));
      if (!scan_done_.is_valid()) {
         return arrow::Status::OK();
      }
      scheduleNextBatch();
      return arrow::Status::OK();
   }

   void PauseProducing(arrow::acero::ExecNode* /*output*/, int32_t counter) override {
      const std::scoped_lock lock(mutex_);
      if (counter <= backpressure_counter_) {
         return;
      }
      backpressure_counter_ = counter;
      paused_ = true;
   }

   void ResumeProducing(arrow::acero::ExecNode* /*output*/, int32_t counter) override {
      {
         const std::scoped_lock lock(mutex_);
         if (counter <= backpressure_counter_) {
            return;
         }
         backpressure_counter_ = counter;
         paused_ = false;
         if (!parked_) {
            return;
         }
         parked_ = false;
      }
      scheduleNextBatch();
   }

  protected:
   arrow::Status StopProducingImpl() override {
      {
         const std::scoped_lock lock(mutex_);
         stopped_ = true;
         if (!parked_) {
            return arrow::Status::OK();
         }
         parked_ = false;
      }
      return finish();
   }

  private:
   // Batches are produced one after another, each in its own task. The next task is scheduled
   // before the current batch is delivered, so producing and consuming overlap.
   void scheduleNextBatch() {
      auto* executor = plan_->query_context()->executor();
      const bool scheduled = plan_->query_context()->async_scheduler()->AddSimpleTask(
         [this, executor] {
            return executor->Submit([this] {
               arrow::Status status;
               try {
                  status = produceNextBatch();
               } catch (const std::exception& exception) {
                  status = arrow::Status::ExecutionError(exception.what());
               }
               if (!status.ok()) {
                  markDone(status);
               }
               return status;
            });
         },
         std::string_view{"TableScan::ProduceBatch"}
      );
      // The plan was aborted
      if (!scheduled) {
         markDone(arrow::Status::OK());
      }
   }

   arrow::Status produceNextBatch() {
      EVOBENCH_SCOPE("TableScan", "produceNextBatch");
      if (!bitmap_reader_.has_value()) {
         bitmap_reader_.emplace(filter_.evaluate().toRoaring(), batch_size_cutoff_);
      }
      bool stopped = false;
      {
         const std::scoped_lock lock(mutex_);
         stopped = stopped_;
      }
      std::optional<roaring::Roaring> row_ids =
         stopped ? std::nullopt : bitmap_reader_->nextBatch();
      if (!row_ids.has_value()) {
         return finish();
      }

      const Bitmap batch_row_ids{std::move(row_ids.value())};
      const auto length = static_cast<int64_t>(batch_row_ids.cardinality());
      ARROW_RETURN_NOT_OK(exec_batch_builder_.appendEntries(*table_, batch_row_ids));
      ARROW_ASSIGN_OR_RAISE(auto batch, exec_batch_builder_.finishBatch(length));
      batch.index = batch_count_++;

      bool parked = false;
      {
         const std::scoped_lock lock(mutex_);
         parked = paused_ && !stopped_;
         parked_ = parked;
      }
      if (!parked) {
         scheduleNextBatch();
      }
      return output_->InputReceived(this, std::move(batch));
   }

   arrow::Status finish() {
      ARROW_RETURN_NOT_OK(output_->InputFinished(this, batch_count_));
      markDone(arrow::Status::OK());
      return arrow::Status::OK();
   }

   void markDone(const arrow::Status& status) {
      if (!done_.exchange(true)) {
         scan_done_.MarkFinished(status);
      }
   }

   ExecBatchBuilder exec_batch_builder_;
   CompiledFilter filter_;
   std::shared_ptr<const storage::Table> table_;
   size_t batch_size_cutoff_;

   // Only touched by the batch tasks, which run one after another
   std::optional<BatchedBitmapReader> bitmap_reader_;
   int batch_count_ = 0;

   arrow::Future<> scan_done_;
   std::atomic<bool> done_ = false;

   std::mutex mutex_;
   int32_t backpressure_counter_ = 0;
   bool paused_ = false;
   // Paused with no batch task scheduled; resuming or stopping continues from here
   bool parked_ = false;
   bool stopped_ = false;
};

}  // namespace

arrow::Result<arrow::acero::ExecNode*> makeTableScan(
   arrow::acero::ExecPlan* plan,
   const std::vector<schema::ColumnIdentifier>& columns,
   CompiledFilter filter,
   std::shared_ptr<const storage::Table> table,
   size_t batch_size_cutoff
) {
   return plan->EmplaceNode<TableScan>(
      plan, columns, std::move(filter), std::move(table), batch_size_cutoff
   );
}

}  // namespace rhydb::query_engine::exec_node
