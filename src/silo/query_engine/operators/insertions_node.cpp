#include "silo/query_engine/operators/insertions_node.h"

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <boost/container_hash/hash.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/schema_output_builder.h"
#include "silo/query_engine/operators/compute_partition_filters.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/insertion_index.h"
#include "silo/storage/table.h"
#include "silo/storage/table_partition.h"

// PositionAndInsertionKey needs std::hash specialization in global namespace,
// so we define the struct and its hash here before any namespace.

namespace silo::query_engine::operators {
struct PositionAndInsertionKey {
   uint32_t position_idx;
   std::string_view insertion_value;

   bool operator==(const PositionAndInsertionKey& other) const {
      return position_idx == other.position_idx && insertion_value == other.insertion_value;
   }
};
}  // namespace silo::query_engine::operators

template <>
struct std::hash<silo::query_engine::operators::PositionAndInsertionKey> {
   std::size_t operator()(const silo::query_engine::operators::PositionAndInsertionKey& key
   ) const noexcept {
      std::size_t seed = 0;
      boost::hash_combine(seed, key.position_idx);
      boost::hash_combine(seed, key.insertion_value);
      return seed;
   }
};

namespace silo::query_engine::operators {

namespace {

template <typename SymbolType>
struct InsertionsPrefilteredBitmaps {
   std::vector<std::pair<
      const CopyOnWriteBitmap&,
      const silo::storage::insertion::InsertionIndex<SymbolType>&>>
      bitmaps;
   std::vector<std::pair<size_t, const silo::storage::insertion::InsertionIndex<SymbolType>&>>
      full_bitmaps;
};

template <typename SymbolType>
std::unordered_map<std::string, InsertionsPrefilteredBitmaps<SymbolType>>
insertionsPreFilterBitmaps(
   const storage::Table& table,
   const std::vector<schema::ColumnIdentifier>& sequence_columns,
   std::vector<CopyOnWriteBitmap>& bitmap_filter
) {
   std::unordered_map<std::string, InsertionsPrefilteredBitmaps<SymbolType>> pre_filtered_bitmaps;
   for (size_t i = 0; i < table.getNumberOfPartitions(); ++i) {
      auto table_partition = table.getPartition(i);

      for (const auto& column_identifier : sequence_columns) {
         const auto& sequence_column =
            table_partition->columns.getColumns<typename SymbolType::Column>().at(
               column_identifier.name
            );
         CopyOnWriteBitmap& filter = bitmap_filter[i];
         const size_t cardinality = filter.getConstReference().cardinality();
         if (cardinality == 0) {
            continue;
         }
         if (cardinality == table_partition->sequence_count) {
            pre_filtered_bitmaps[column_identifier.name].full_bitmaps.emplace_back(
               cardinality, sequence_column.insertion_index
            );
         } else {
            if (filter.isMutable()) {
               filter.getMutable().runOptimize();
            }
            pre_filtered_bitmaps[column_identifier.name].bitmaps.emplace_back(
               filter, sequence_column.insertion_index
            );
         }
      }
   }
   return pre_filtered_bitmaps;
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Status addAggregatedInsertionsToInsertionCounts(
   const std::string& sequence_name,
   bool show_sequence_in_response,
   const InsertionsPrefilteredBitmaps<SymbolType>& prefiltered_bitmaps,
   exec_node::SchemaOutputBuilder& output_builder
) {
   std::unordered_map<PositionAndInsertionKey, uint32_t> all_insertions;
   for (const auto& [_, insertion_index] : prefiltered_bitmaps.full_bitmaps) {
      for (const auto& [position, insertions_at_position] :
           insertion_index.getInsertionPositions()) {
         for (const auto& insertion : insertions_at_position.insertions) {
            all_insertions[PositionAndInsertionKey{position, insertion.value}] +=
               insertion.row_ids.cardinality();
         }
      }
   }
   for (const auto& [bitmap_filter, insertion_index] : prefiltered_bitmaps.bitmaps) {
      for (const auto& [position, insertions_at_position] :
           insertion_index.getInsertionPositions()) {
         for (const auto& insertion : insertions_at_position.insertions) {
            const uint32_t count =
               insertion.row_ids.and_cardinality(bitmap_filter.getConstReference());
            if (count > 0) {
               all_insertions[PositionAndInsertionKey{position, insertion.value}] += count;
            }
         }
      }
   }
   const std::string sequence_in_response = show_sequence_in_response ? sequence_name + ":" : "";
   for (const auto& [position_and_insertion, count] : all_insertions) {
      using OutputValue = std::optional<std::variant<std::string, bool, int32_t, double>>;
      // TODO.TAE remove again
      SPDLOG_INFO(
         "output_builder.addValueIfContainedInOutput {}", position_and_insertion.position_idx
      );
      ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
         InsertionsNode<SymbolType>::POSITION_FIELD_NAME,
         [&]() -> OutputValue { return static_cast<int32_t>(position_and_insertion.position_idx); }
      ));
      ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
         InsertionsNode<SymbolType>::INSERTED_SYMBOLS_FIELD_NAME,
         [&]() -> OutputValue { return std::string(position_and_insertion.insertion_value); }
      ));
      ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
         InsertionsNode<SymbolType>::SEQUENCE_FIELD_NAME,
         [&]() -> OutputValue { return sequence_name; }
      ));
      ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
         InsertionsNode<SymbolType>::INSERTION_FIELD_NAME,
         [&]() -> OutputValue {
            return fmt::format(
               "ins_{}{}:{}",
               sequence_in_response,
               position_and_insertion.position_idx,
               position_and_insertion.insertion_value
            );
         }
      ));
      ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
         InsertionsNode<SymbolType>::COUNT_FIELD_NAME,
         [&]() -> OutputValue { return static_cast<int32_t>(count); }
      ));
   }
   return arrow::Status::OK();
}

}  // namespace

template <typename SymbolType>
arrow::Result<PartialArrowPlan> InsertionsNode<SymbolType>::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   auto partition_filters = computePartitionFilters(filter, *table);

   auto table_handle = table;
   auto sequence_columns_handle = sequence_columns;
   auto output_fields = getOutputSchema();
   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      // NOLINTNEXTLINE(readability-function-cognitive-complexity)
      [table_handle,
       output_fields,
       partition_filters,
       sequence_columns_handle,
       already_produced = false]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (already_produced) {
         const std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      already_produced = true;

      exec_node::SchemaOutputBuilder output_builder{output_fields};

      const auto bitmaps_to_evaluate = insertionsPreFilterBitmaps<SymbolType>(
         *table_handle, sequence_columns_handle, partition_filters
      );
      for (const auto& [sequence_name, prefiltered_bitmaps] : bitmaps_to_evaluate) {
         const auto default_sequence_name =
            table_handle->schema.template getDefaultSequenceName<SymbolType>();
         const bool omit_sequence_in_response =
            default_sequence_name.has_value() &&
            (default_sequence_name.value().name == sequence_name);
         ARROW_RETURN_NOT_OK(addAggregatedInsertionsToInsertionCounts<SymbolType>(
            sequence_name, !omit_sequence_in_response, prefiltered_bitmaps, output_builder
         ));
      }
      SPDLOG_INFO("output_schema:");
      for (const auto& field : output_fields) {
         SPDLOG_INFO("field: {}", field.name);
      }

      ARROW_ASSIGN_OR_RAISE(std::vector<arrow::Datum> result_columns, output_builder.finish());
      ARROW_ASSIGN_OR_RAISE(
         const std::optional<arrow::ExecBatch> result, arrow::ExecBatch::Make(result_columns)
      );
      return arrow::Future{result};
   };

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   const arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(output_fields),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   ARROW_ASSIGN_OR_RAISE(
      auto node, arrow::acero::MakeExecNode("source", arrow_plan.get(), {}, options)
   );

   return PartialArrowPlan{node, arrow_plan};
}

template class InsertionsNode<Nucleotide>;
template class InsertionsNode<AminoAcid>;

}  // namespace silo::query_engine::operators
