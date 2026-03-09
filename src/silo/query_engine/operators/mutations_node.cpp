#include "silo/query_engine/operators/mutations_node.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

#include <arrow/acero/exec_plan.h>
#include <arrow/acero/options.h>
#include <fmt/format.h>

#include "evobench/evobench.hpp"
#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/schema_output_builder.h"
#include "silo/query_engine/operators/compute_partition_filters.h"
#include "silo/schema/database_schema.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table.h"
#include "silo/storage/table_partition.h"

namespace silo::query_engine::operators {

namespace {

template <typename SymbolType>
struct MutationsPrefilteredBitmaps {
   std::vector<std::pair<
      const CopyOnWriteBitmap&,
      const storage::column::SequenceColumnPartition<SymbolType>&>>
      bitmaps;
   std::vector<std::pair<size_t, const storage::column::SequenceColumnPartition<SymbolType>&>>
      full_bitmaps;
};

template <typename SymbolType>
std::unordered_map<std::string, MutationsPrefilteredBitmaps<SymbolType>> mutationsPreFilterBitmaps(
   const silo::storage::Table& table,
   std::vector<CopyOnWriteBitmap>& bitmap_filter
) {
   std::unordered_map<std::string, MutationsPrefilteredBitmaps<SymbolType>> bitmaps_to_evaluate;
   for (size_t i = 0; i < table.getNumberOfPartitions(); ++i) {
      auto table_partition = table.getPartition(i);
      CopyOnWriteBitmap& filter = bitmap_filter[i];
      const size_t cardinality = filter.getConstReference().cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == table_partition->sequence_count) {
         for (const auto& [sequence_name, sequence_store] :
              table_partition->columns.getColumns<typename SymbolType::Column>()) {
            bitmaps_to_evaluate[sequence_name].full_bitmaps.emplace_back(
               cardinality, sequence_store
            );
         }
      } else {
         if (filter.isMutable()) {
            filter.getMutable().runOptimize();
         }
         for (const auto& [sequence_name, sequence_store] :
              table_partition->columns.getColumns<typename SymbolType::Column>()) {
            bitmaps_to_evaluate[sequence_name].bitmaps.emplace_back(filter, sequence_store);
         }
      }
   }
   return bitmaps_to_evaluate;
}

using silo::storage::column::VerticalSequenceIndex;

template <typename SymbolType>
using SequenceDiffKey = typename VerticalSequenceIndex<SymbolType>::SequenceDiffKey;

template <typename SymbolType>
using SequenceDiff = typename VerticalSequenceIndex<SymbolType>::SequenceDiff;

__attribute__((noinline)) void initializeCountsWithSequenceCount(
   std::vector<uint32_t>& count_per_local_reference_position,
   uint32_t sequence_count
) {
   EVOBENCH_SCOPE("Mutations", "initializeCountsWithSequenceCount");
   // NOLINTNEXTLINE(modernize-loop-convert)
   for (uint32_t position_idx = 0; position_idx < count_per_local_reference_position.size();
        ++position_idx) {
      count_per_local_reference_position[position_idx] += sequence_count;
   }
}

__attribute__((noinline)) void subtractHorizontalBitmapCounts(
   std::vector<uint32_t>& count_per_local_reference_position,
   const std::map<uint32_t, roaring::Roaring>& horizontal_bitmaps
) {
   EVOBENCH_SCOPE("Mutations", "subtractHorizontalBitmapCounts");
   for (const auto& [_, n_bitmap] : horizontal_bitmaps) {
      for (const uint32_t position_idx : n_bitmap) {
         count_per_local_reference_position[position_idx] -= 1;
      }
   }
}

__attribute__((noinline)) void subtractCumulativeNsFromPositions(
   std::vector<uint32_t>& count_per_local_reference_position,
   uint32_t sequence_length,
   const std::vector<size_t>& cumulative_starts,
   const std::vector<size_t>& cumulative_ends
) {
   EVOBENCH_SCOPE("Mutations", "subtractCumulativeNsFromPositions");
   size_t running_total_start_n_offset = cumulative_starts.at(sequence_length);
   size_t start_position_iter = sequence_length - 1;
   while (true) {
      count_per_local_reference_position.at(start_position_iter) -= running_total_start_n_offset;
      running_total_start_n_offset += cumulative_starts.at(start_position_iter);
      if (start_position_iter == 0) {
         break;
      }
      start_position_iter -= 1;
   }
   size_t running_total_end_n_offset = cumulative_ends.at(0);
   size_t end_position_iter = 0;
   while (true) {
      count_per_local_reference_position.at(end_position_iter) -= running_total_end_n_offset;
      running_total_end_n_offset += cumulative_ends.at(end_position_iter + 1);
      if (end_position_iter == sequence_length - 1) {
         break;
      }
      end_position_iter += 1;
   }
}

__attribute__((noinline)) void subtractStartAndEndNCounts(
   std::vector<uint32_t>& count_per_local_reference_position,
   const std::vector<std::pair<uint32_t, uint32_t>>& start_end,
   size_t sequence_length
) {
   EVOBENCH_SCOPE("Mutations", "subtractStartAndEndNCounts");
   std::vector<size_t> cumulative_starts(sequence_length + 1);
   std::vector<size_t> cumulative_ends(sequence_length + 1);
   for (const auto& [start, end] : start_end) {
      cumulative_starts.at(start) += 1;
      cumulative_ends.at(end) += 1;
   }
   subtractCumulativeNsFromPositions(
      count_per_local_reference_position, sequence_length, cumulative_starts, cumulative_ends
   );
}

__attribute__((noinline)) void subtractFilteredNCounts(
   std::vector<uint32_t>& count_per_local_reference_position,
   const CopyOnWriteBitmap& filter,
   size_t sequence_length,
   const std::map<uint32_t, roaring::Roaring>& horizontal_bitmaps,
   const std::vector<std::pair<uint32_t, uint32_t>>& start_end
) {
   EVOBENCH_SCOPE("Mutations", "subtractFilteredNCounts");
   std::vector<size_t> cumulative_starts(sequence_length + 1);
   std::vector<size_t> cumulative_ends(sequence_length + 1);
   for (const uint32_t idx : filter.getConstReference()) {
      auto iter = horizontal_bitmaps.find(idx);
      if (iter != horizontal_bitmaps.end()) {
         const roaring::Roaring& n_bitmap = iter->second;
         for (const size_t position_idx : n_bitmap) {
            count_per_local_reference_position[position_idx] -= 1;
         }
      }
      auto [start, end] = start_end.at(idx);
      cumulative_starts.at(start) += 1;
      cumulative_ends.at(end) += 1;
   }
   subtractCumulativeNsFromPositions(
      count_per_local_reference_position, sequence_length, cumulative_starts, cumulative_ends
   );
}

template <typename SymbolType>
void countActualMutations(
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position,
   std::vector<uint32_t>& count_per_local_reference_position,
   const std::map<SequenceDiffKey<SymbolType>, SequenceDiff<SymbolType>>& vertical_bitmaps
) {
   EVOBENCH_SCOPE("Mutations", "countActualMutations");
   for (const auto& [sequence_diff_key, sequence_diff] : vertical_bitmaps) {
      count_of_mutations_per_position[sequence_diff_key.symbol][sequence_diff_key.position] +=
         sequence_diff.cardinality;
      count_per_local_reference_position[sequence_diff_key.position] -= sequence_diff.cardinality;
   }
}

template <typename SymbolType>
void countActualFilteredMutations(
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position,
   std::vector<uint32_t>& count_per_local_reference_position,
   const CopyOnWriteBitmap& filter,
   const std::map<SequenceDiffKey<SymbolType>, SequenceDiff<SymbolType>>& vertical_bitmaps
) {
   EVOBENCH_SCOPE("Mutations", "countActualFilteredMutations");
   const auto& filter_roaring_array = filter.getConstReference().roaring.high_low_container;
   std::map<size_t, roaring::internal::container_t*> filter_containers;
   std::map<size_t, uint8_t> filter_container_typecodes;
   for (int32_t idx = 0; idx < filter_roaring_array.size; ++idx) {
      filter_containers[filter_roaring_array.keys[idx]] = filter_roaring_array.containers[idx];
      filter_container_typecodes[filter_roaring_array.keys[idx]] =
         filter_roaring_array.typecodes[idx];
   }

   for (const auto& [sequence_diff_key, sequence_diff] : vertical_bitmaps) {
      auto iter = filter_containers.find(sequence_diff_key.v_index);
      if (iter != filter_containers.end()) {
         auto filter_container = iter->second;
         const uint8_t filter_container_typecode =
            filter_container_typecodes.at(sequence_diff_key.v_index);

         auto contained_count = roaring::internal::container_and_cardinality(
            filter_container,
            filter_container_typecode,
            sequence_diff.container,
            sequence_diff.typecode
         );

         count_of_mutations_per_position[sequence_diff_key.symbol][sequence_diff_key.position] +=
            contained_count;
         count_per_local_reference_position[sequence_diff_key.position] -= contained_count;
      }
   }
}

template <typename SymbolType>
__attribute__((noinline)) void accumulateFinalCounts(
   const std::vector<uint32_t>& count_per_local_reference_position,
   const std::vector<typename SymbolType::Symbol>& local_reference,
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   EVOBENCH_SCOPE("Mutations", "accumulateFinalCounts");
   for (size_t position_idx = 0; position_idx < count_per_local_reference_position.size();
        ++position_idx) {
      count_of_mutations_per_position[local_reference.at(position_idx)][position_idx] +=
         count_per_local_reference_position[position_idx];
   }
}

template <typename SymbolType>
void addMutationCountsForMixedBitmaps(
   const MutationsPrefilteredBitmaps<SymbolType>& bitmaps_to_evaluate,
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   for (const auto& [filter, sequence_column_partition] : bitmaps_to_evaluate.bitmaps) {
      auto local_reference = sequence_column_partition.getLocalReference();
      const size_t sequence_length = local_reference.size();
      std::vector<uint32_t> count_per_local_reference_position(sequence_length);

      initializeCountsWithSequenceCount(
         count_per_local_reference_position, filter.getConstReference().cardinality()
      );
      subtractFilteredNCounts(
         count_per_local_reference_position,
         filter,
         sequence_length,
         sequence_column_partition.horizontal_coverage_index.horizontal_bitmaps,
         sequence_column_partition.horizontal_coverage_index.start_end
      );
      countActualFilteredMutations(
         count_of_mutations_per_position,
         count_per_local_reference_position,
         filter,
         sequence_column_partition.vertical_sequence_index.vertical_bitmaps
      );
      accumulateFinalCounts(
         count_per_local_reference_position, local_reference, count_of_mutations_per_position
      );
   }
}

template <typename SymbolType>
void addMutationCountsForFullBitmaps(
   const MutationsPrefilteredBitmaps<SymbolType>& bitmaps_to_evaluate,
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   for (const auto& [_, sequence_column_partition] : bitmaps_to_evaluate.full_bitmaps) {
      auto local_reference = sequence_column_partition.getLocalReference();
      const size_t sequence_length = local_reference.size();
      std::vector<uint32_t> count_per_local_reference_position(sequence_length);

      initializeCountsWithSequenceCount(
         count_per_local_reference_position, sequence_column_partition.sequence_count
      );
      subtractHorizontalBitmapCounts(
         count_per_local_reference_position,
         sequence_column_partition.horizontal_coverage_index.horizontal_bitmaps
      );
      subtractStartAndEndNCounts(
         count_per_local_reference_position,
         sequence_column_partition.horizontal_coverage_index.start_end,
         sequence_length
      );
      countActualMutations(
         count_of_mutations_per_position,
         count_per_local_reference_position,
         sequence_column_partition.vertical_sequence_index.vertical_bitmaps
      );
      accumulateFinalCounts(
         count_per_local_reference_position, local_reference, count_of_mutations_per_position
      );
   }
}

template <typename SymbolType>
SymbolMap<SymbolType, std::vector<uint32_t>> calculateMutationsPerPosition(
   const storage::column::SequenceColumnMetadata<SymbolType>& column_metadata,
   const MutationsPrefilteredBitmaps<SymbolType>& bitmap_filter
) {
   const size_t sequence_length = column_metadata.reference_sequence.size();
   SymbolMap<SymbolType, std::vector<uint32_t>> count_of_mutations_per_position;
   for (const auto symbol : SymbolType::SYMBOLS) {
      count_of_mutations_per_position[symbol] = std::vector<uint32_t>(sequence_length, 0);
   }
   addMutationCountsForMixedBitmaps<SymbolType>(bitmap_filter, count_of_mutations_per_position);
   addMutationCountsForFullBitmaps<SymbolType>(bitmap_filter, count_of_mutations_per_position);
   return count_of_mutations_per_position;
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Status addMutationsToOutput(
   const std::string& sequence_name,
   const storage::column::SequenceColumnMetadata<SymbolType>& column_metadata,
   double min_proportion,
   const MutationsPrefilteredBitmaps<SymbolType>& bitmap_filter,
   exec_node::SchemaOutputBuilder& output_builder
) {
   const uint32_t sequence_length = column_metadata.reference_sequence.size();

   const SymbolMap<SymbolType, std::vector<uint32_t>> count_of_mutations_per_position =
      calculateMutationsPerPosition<SymbolType>(column_metadata, bitmap_filter);

   for (uint32_t pos = 0; pos < sequence_length; ++pos) {
      uint32_t total = 0;
      for (const typename SymbolType::Symbol symbol : SymbolType::VALID_MUTATION_SYMBOLS) {
         total += count_of_mutations_per_position.at(symbol)[pos];
      }
      if (total == 0) {
         continue;
      }
      const auto threshold_count =
         min_proportion == 0
            ? 0
            : static_cast<uint32_t>(std::ceil(static_cast<double>(total) * min_proportion) - 1);

      const typename SymbolType::Symbol symbol_in_reference_genome =
         column_metadata.reference_sequence.at(pos);

      for (const auto symbol : SymbolType::VALID_MUTATION_SYMBOLS) {
         if (symbol_in_reference_genome != symbol) {
            const uint32_t count = count_of_mutations_per_position.at(symbol)[pos];
            if (count > threshold_count) {
               const double proportion = static_cast<double>(count) / static_cast<double>(total);
               using OutputValue = std::optional<std::variant<std::string, bool, int32_t, double>>;
               ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
                  MutationsNode<SymbolType>::MUTATION_FIELD_NAME,
                  [&]() -> OutputValue {
                     return {fmt::format(
                        "{}{}{}",
                        SymbolType::symbolToChar(symbol_in_reference_genome),
                        pos + 1,
                        SymbolType::symbolToChar(symbol)
                     )};
                  }
               ));
               ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
                  MutationsNode<SymbolType>::MUTATION_FROM_FIELD_NAME,
                  [&]() -> OutputValue {
                     return {std::string(1, SymbolType::symbolToChar(symbol_in_reference_genome))};
                  }
               ));
               ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
                  MutationsNode<SymbolType>::MUTATION_TO_FIELD_NAME,
                  [&]() -> OutputValue {
                     return {std::string(1, SymbolType::symbolToChar(symbol))};
                  }
               ));
               ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
                  MutationsNode<SymbolType>::POSITION_FIELD_NAME,
                  [&]() -> OutputValue { return {static_cast<int32_t>(pos + 1)}; }
               ));
               ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
                  MutationsNode<SymbolType>::SEQUENCE_FIELD_NAME,
                  [&]() -> OutputValue { return {sequence_name}; }
               ));
               ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
                  MutationsNode<SymbolType>::PROPORTION_FIELD_NAME,
                  [&]() -> OutputValue { return {proportion}; }
               ));
               ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
                  MutationsNode<SymbolType>::COUNT_FIELD_NAME,
                  [&]() -> OutputValue { return {static_cast<int32_t>(count)}; }
               ));
               ARROW_RETURN_NOT_OK(output_builder.addValueIfContainedInOutput(
                  MutationsNode<SymbolType>::COVERAGE_FIELD_NAME,
                  [&]() -> OutputValue { return {static_cast<int32_t>(total)}; }
               ));
            }
         }
      }
   }
   return arrow::Status::OK();
}

}  // namespace

template <typename SymbolType>
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
arrow::Result<PartialArrowPlan> MutationsNode<SymbolType>::toQueryPlan(
   const std::map<schema::TableName, std::shared_ptr<storage::Table>>& /*tables*/,
   const config::QueryOptions& /*query_options*/
) const {
   auto partition_filters = computePartitionFilters(filter, *table);

   auto table_handle = table;
   auto output_fields = getOutputSchema();
   auto sequence_columns_handle = sequence_columns;
   const double given_min_proportion = min_proportion;
   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      // NOLINTNEXTLINE(readability-function-cognitive-complexity)
      [table_handle,
       given_min_proportion,
       output_fields,
       partition_filters,
       sequence_columns_handle,
       already_produced = false]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      if (already_produced) {
         const std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      already_produced = true;

      auto bitmaps_to_evaluate =
         mutationsPreFilterBitmaps<SymbolType>(*table_handle, partition_filters);

      exec_node::SchemaOutputBuilder output_builder(output_fields);

      for (const auto& sequence_column : sequence_columns_handle) {
         const storage::column::SequenceColumnMetadata<SymbolType>* sequence_column_metadata =
            table_handle->schema
               ->template getColumnMetadata<typename SymbolType::Column>(sequence_column.name)
               .value();

         if (bitmaps_to_evaluate.contains(sequence_column.name)) {
            ARROW_RETURN_NOT_OK(addMutationsToOutput<SymbolType>(
               sequence_column.name,
               *sequence_column_metadata,
               given_min_proportion,
               bitmaps_to_evaluate.at(sequence_column.name),
               output_builder
            ));
         }
      }
      ARROW_ASSIGN_OR_RAISE(
         const std::vector<arrow::Datum> result_columns, output_builder.finish()
      );
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

   return PartialArrowPlan{.top_node = node, .plan = arrow_plan};
}

template class MutationsNode<Nucleotide>;
template class MutationsNode<AminoAcid>;

}  // namespace silo::query_engine::operators
