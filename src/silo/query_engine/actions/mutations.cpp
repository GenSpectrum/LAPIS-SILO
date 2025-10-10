#include "silo/query_engine/actions/mutations.h"

#include <cmath>
#include <cstddef>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <arrow/util/future.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/query_engine/exec_node/json_value_type_array_builder.h"
#include "silo/storage/column/sequence_column.h"
#include "silo/storage/table_partition.h"

using silo::query_engine::CopyOnWriteBitmap;

namespace silo::query_engine::actions {

template <typename SymbolType>
Mutations<SymbolType>::Mutations(
   std::vector<std::string>&& sequence_names,
   double min_proportion,
   std::vector<std::string_view>&& fields
)
    : sequence_names(std::move(sequence_names)),
      min_proportion(min_proportion),
      fields(std::move(fields)) {
   if (this->fields.empty()) {
      this->fields = std::vector<std::string_view>{VALID_FIELDS.begin(), VALID_FIELDS.end()};
   }
}

template <typename SymbolType>
std::unordered_map<std::string, typename Mutations<SymbolType>::PrefilteredBitmaps> Mutations<
   SymbolType>::
   preFilterBitmaps(
      const silo::storage::Table& table,
      std::vector<CopyOnWriteBitmap>& bitmap_filter
   ) {
   std::unordered_map<std::string, PrefilteredBitmaps> bitmaps_to_evaluate;
   for (size_t i = 0; i < table.getNumberOfPartitions(); ++i) {
      const storage::TablePartition& table_partition = table.getPartition(i);
      CopyOnWriteBitmap& filter = bitmap_filter[i];
      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == table_partition.sequence_count) {
         for (const auto& [sequence_name, sequence_store] :
              table_partition.columns.getColumns<typename SymbolType::Column>()) {
            bitmaps_to_evaluate[sequence_name].full_bitmaps.emplace_back(
               cardinality, sequence_store
            );
         }
      } else {
         if (filter.isMutable()) {
            filter->runOptimize();
         }
         for (const auto& [sequence_name, sequence_store] :
              table_partition.columns.getColumns<typename SymbolType::Column>()) {
            bitmaps_to_evaluate[sequence_name].bitmaps.emplace_back(filter, sequence_store);
         }
      }
   }
   return bitmaps_to_evaluate;
}

namespace {

__attribute__((noinline)) void initializeCountsWithSequenceCount(
   std::vector<uint32_t>& count_per_local_reference_position,
   uint32_t sequence_count
) {
   for (size_t position_idx = 0; position_idx < count_per_local_reference_position.size();
        ++position_idx) {
      count_per_local_reference_position[position_idx] += sequence_count;
   }
}

__attribute__((noinline)) void subtractHorizontalBitmapCounts(
   std::vector<uint32_t>& count_per_local_reference_position,
   const std::map<size_t, roaring::Roaring>& horizontal_bitmaps
) {
   for (const auto& [_, n_bitmap] : horizontal_bitmaps) {
      for (size_t position_idx : n_bitmap) {
         count_per_local_reference_position[position_idx] -= 1;
      }
   }
}

void substractCumulativeNsFromPositions(
   std::vector<uint32_t>& count_per_local_reference_position,
   size_t sequence_length,
   const std::vector<size_t>& cumulative_starts,
   const std::vector<size_t>& cumulative_ends
) {
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
   // Indexes are not symmetric to start_n, because end is exclusive!
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

__attribute__((noinline)) void substractStartAndEndNCounts(
   std::vector<uint32_t>& count_per_local_reference_position,
   const std::vector<std::pair<size_t, size_t>>& start_end,
   size_t sequence_length
) {
   std::vector<size_t> cumulative_starts(sequence_length + 1);
   std::vector<size_t> cumulative_ends(sequence_length + 1);
   for (const auto& [start, end] : start_end) {
      cumulative_starts.at(start) += 1;
      cumulative_ends.at(end) += 1;
   }
   substractCumulativeNsFromPositions(
      count_per_local_reference_position, sequence_length, cumulative_starts, cumulative_ends
   );
}

__attribute__((noinline)) void substractFilteredNCounts(
   std::vector<uint32_t>& count_per_local_reference_position,
   const CopyOnWriteBitmap& filter,
   size_t sequence_length,
   const std::map<size_t, roaring::Roaring>& horizontal_bitmaps,
   const std::vector<std::pair<size_t, size_t>>& start_end
) {
   std::vector<size_t> cumulative_starts(sequence_length + 1);
   std::vector<size_t> cumulative_ends(sequence_length + 1);
   for (const uint32_t idx : *filter) {
      auto iter = horizontal_bitmaps.find(idx);
      if (iter != horizontal_bitmaps.end()) {
         const roaring::Roaring& n_bitmap = iter->second;
         for (size_t position_idx : n_bitmap) {
            count_per_local_reference_position[position_idx] -= 1;
         }
      }
      auto [start, end] = start_end.at(idx);
      cumulative_starts.at(start) += 1;
      cumulative_ends.at(end) += 1;
   }
   substractCumulativeNsFromPositions(
      count_per_local_reference_position, sequence_length, cumulative_starts, cumulative_ends
   );
}

using silo::storage::column::SequenceColumnPartition;
using silo::storage::column::VerticalSequenceIndex;

template <typename SymbolType>
using SequenceDiffKey = typename VerticalSequenceIndex<SymbolType>::SequenceDiffKey;

template <typename SymbolType>
using SequenceDiff = typename VerticalSequenceIndex<SymbolType>::SequenceDiff;

template <typename SymbolType>
void countActualMutations(
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position,
   std::vector<uint32_t>& count_per_local_reference_position,
   const std::map<SequenceDiffKey<SymbolType>, SequenceDiff<SymbolType>>& vertical_bitmaps
) {
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
   const auto& filter_roaring_array = filter->roaring.high_low_container;
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
         uint8_t filter_container_typecode =
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
   for (size_t position_idx = 0; position_idx < count_per_local_reference_position.size();
        ++position_idx) {
      count_of_mutations_per_position[local_reference.at(position_idx)][position_idx] +=
         count_per_local_reference_position[position_idx];
   }
}

template <typename SymbolType>
__attribute__((noinline)) void logIt(
   const std::vector<uint32_t>& count_per_local_reference_position,
   const std::vector<typename SymbolType::Symbol>& local_reference,
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   std::string header = "  ";
   SymbolMap<SymbolType, std::string> symbol_lines;

   for (const auto& symbol : SymbolType::VALID_MUTATION_SYMBOLS) {
      symbol_lines[symbol] = fmt::format("{} |", SymbolType::symbolToChar(symbol));
   }

   for (size_t position_idx = 0; position_idx < count_per_local_reference_position.size();
        ++position_idx) {
      header += fmt::format("{:>10}", position_idx);
      for (const auto& symbol : SymbolType::VALID_MUTATION_SYMBOLS) {
         size_t count;
         if (symbol == local_reference.at(position_idx)) {
            count = count_of_mutations_per_position[symbol][position_idx] +
                    count_per_local_reference_position[position_idx];
         } else {
            count = count_of_mutations_per_position[symbol][position_idx];
         }
         symbol_lines[symbol] += fmt::format("{:>10}", count);
      }
   }

   SPDLOG_INFO(header);

   for (const auto& symbol : SymbolType::VALID_MUTATION_SYMBOLS) {
      SPDLOG_INFO(symbol_lines[symbol]);
   }
}

}  // anonymous namespace

template <typename SymbolType>
void Mutations<SymbolType>::addMutationCountsForMixedBitmaps(
   const PrefilteredBitmaps& bitmaps_to_evaluate,
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   for (const auto& [filter, sequence_column_partition] : bitmaps_to_evaluate.bitmaps) {
      auto local_reference = sequence_column_partition.getLocalReference();
      size_t sequence_length = local_reference.size();
      std::vector<uint32_t> count_per_local_reference_position(sequence_length);

      // SPDLOG_INFO("start");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      initializeCountsWithSequenceCount(count_per_local_reference_position, filter->cardinality());

      // SPDLOG_INFO("after init");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      substractFilteredNCounts(
         count_per_local_reference_position,
         filter,
         sequence_length,
         sequence_column_partition.horizontal_coverage_index.horizontal_bitmaps,
         sequence_column_partition.horizontal_coverage_index.start_end
      );

      // SPDLOG_INFO("after Ns");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      countActualFilteredMutations(
         count_of_mutations_per_position,
         count_per_local_reference_position,
         filter,
         sequence_column_partition.vertical_sequence_index.vertical_bitmaps
      );

      // SPDLOG_INFO("after mutations");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      accumulateFinalCounts(
         count_per_local_reference_position, local_reference, count_of_mutations_per_position
      );
   }
}

template <typename SymbolType>
void Mutations<SymbolType>::addMutationCountsForFullBitmaps(
   const PrefilteredBitmaps& bitmaps_to_evaluate,
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   // For these partitions, we have full bitmaps. Do not need to bother with AND
   // cardinality
   for (const auto& [_, sequence_column_partition] : bitmaps_to_evaluate.full_bitmaps) {
      auto local_reference = sequence_column_partition.getLocalReference();
      size_t sequence_length = local_reference.size();
      std::vector<uint32_t> count_per_local_reference_position(sequence_length);

      // SPDLOG_INFO(".initially");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      initializeCountsWithSequenceCount(
         count_per_local_reference_position, sequence_column_partition.sequence_count
      );

      // SPDLOG_INFO(".after init");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      subtractHorizontalBitmapCounts(
         count_per_local_reference_position,
         sequence_column_partition.horizontal_coverage_index.horizontal_bitmaps
      );

      // SPDLOG_INFO(".after horizontal bitmaps");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      substractStartAndEndNCounts(
         count_per_local_reference_position,
         sequence_column_partition.horizontal_coverage_index.start_end,
         sequence_length
      );

      // SPDLOG_INFO(".after coverage ranges");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      countActualMutations(
         count_of_mutations_per_position,
         count_per_local_reference_position,
         sequence_column_partition.vertical_sequence_index.vertical_bitmaps
      );

      // SPDLOG_INFO(".after mutations");
      // logIt(count_per_local_reference_position, local_reference,
      // count_of_mutations_per_position);

      accumulateFinalCounts(
         count_per_local_reference_position, local_reference, count_of_mutations_per_position
      );
   }
}

#undef NOINLINE

template <typename SymbolType>
SymbolMap<SymbolType, std::vector<uint32_t>> Mutations<SymbolType>::calculateMutationsPerPosition(
   const storage::column::SequenceColumnMetadata<SymbolType>& metadata,
   const PrefilteredBitmaps& bitmap_filter
) {
   const size_t sequence_length = metadata.reference_sequence.size();

   SymbolMap<SymbolType, std::vector<uint32_t>> count_of_mutations_per_position;
   for (const auto symbol : SymbolType::SYMBOLS) {
      count_of_mutations_per_position[symbol] = std::vector<uint32_t>(sequence_length, 0);
   }
   addMutationCountsForMixedBitmaps(bitmap_filter, count_of_mutations_per_position);
   addMutationCountsForFullBitmaps(bitmap_filter, count_of_mutations_per_position);
   return count_of_mutations_per_position;
}

template <typename SymbolType>
void Mutations<SymbolType>::validateOrderByFields(const schema::TableSchema& /*table_schema*/)
   const {
   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         std::ranges::any_of(
            fields, [&](const std::string_view& result_field) { return result_field == field.name; }
         ),
         "OrderByField {} is not contained in the result of this operation. "
         "Allowed values are {}.",
         field.name,
         fmt::join(fields, ", ")
      );
   }
}

template <typename SymbolType>
arrow::Status Mutations<SymbolType>::addMutationsToOutput(
   const std::string& sequence_name,
   const storage::column::SequenceColumnMetadata<SymbolType>& sequence_column_metadata,
   double min_proportion,
   const PrefilteredBitmaps& bitmap_filter,
   std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder
) {
   const size_t sequence_length = sequence_column_metadata.reference_sequence.size();

   const SymbolMap<SymbolType, std::vector<uint32_t>> count_of_mutations_per_position =
      calculateMutationsPerPosition(sequence_column_metadata, bitmap_filter);

   for (size_t pos = 0; pos < sequence_length; ++pos) {
      uint32_t total = 0;
      for (const typename SymbolType::Symbol symbol : SymbolType::VALID_MUTATION_SYMBOLS) {
         // SPDLOG_INFO(
         //    "count of symbol {} at pos {}: {}",
         //    SymbolType::symbolToChar(symbol),
         //    pos,
         //    count_of_mutations_per_position.at(symbol)[pos]
         // );
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
         sequence_column_metadata.reference_sequence.at(pos);

      for (const auto symbol : SymbolType::VALID_MUTATION_SYMBOLS) {
         if (symbol_in_reference_genome != symbol) {
            const uint32_t count = count_of_mutations_per_position.at(symbol)[pos];
            if (count > threshold_count) {
               const double proportion = static_cast<double>(count) / static_cast<double>(total);
               if (auto builder = output_builder.find(MUTATION_FIELD_NAME);
                   builder != output_builder.end()) {
                  ARROW_RETURN_NOT_OK(builder->second.insert({fmt::format(
                     "{}{}{}",
                     SymbolType::symbolToChar(symbol_in_reference_genome),
                     pos + 1,
                     SymbolType::symbolToChar(symbol)
                  )}));
               }
               if (auto builder = output_builder.find(MUTATION_FROM_FIELD_NAME);
                   builder != output_builder.end()) {
                  ARROW_RETURN_NOT_OK(builder->second.insert(
                     {std::string(1, SymbolType::symbolToChar(symbol_in_reference_genome))}
                  ));
               }
               if (auto builder = output_builder.find(MUTATION_TO_FIELD_NAME);
                   builder != output_builder.end()) {
                  ARROW_RETURN_NOT_OK(
                     builder->second.insert({std::string(1, SymbolType::symbolToChar(symbol))})
                  );
               }
               if (auto builder = output_builder.find(POSITION_FIELD_NAME);
                   builder != output_builder.end()) {
                  ARROW_RETURN_NOT_OK(builder->second.insert({static_cast<int32_t>(pos + 1)}));
               }
               if (auto builder = output_builder.find(SEQUENCE_FIELD_NAME);
                   builder != output_builder.end()) {
                  ARROW_RETURN_NOT_OK(builder->second.insert({sequence_name}));
               }
               if (auto builder = output_builder.find(PROPORTION_FIELD_NAME);
                   builder != output_builder.end()) {
                  ARROW_RETURN_NOT_OK(builder->second.insert({proportion}));
               }
               if (auto builder = output_builder.find(COUNT_FIELD_NAME);
                   builder != output_builder.end()) {
                  ARROW_RETURN_NOT_OK(builder->second.insert({static_cast<int32_t>(count)}));
               }
               if (auto builder = output_builder.find(COVERAGE_FIELD_NAME);
                   builder != output_builder.end()) {
                  ARROW_RETURN_NOT_OK(builder->second.insert({static_cast<int32_t>(total)}));
               }
            }
         }
      }
   }
   return arrow::Status::OK();
}

template <typename SymbolType>
arrow::Result<QueryPlan> Mutations<SymbolType>::toQueryPlanImpl(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> partition_filters,
   const config::QueryOptions& query_options,
   std::string_view request_id
) const {
   EVOBENCH_SCOPE("Mutations", "toQueryPlanImpl");
   std::vector<std::string> sequence_names_to_evaluate;
   for (const auto& sequence_name : sequence_names) {
      auto column_identifier = table->schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column_identifier.has_value() && column_identifier.value().type == SymbolType::COLUMN_TYPE,
         "Database does not contain the {} sequence with name: '{}'",
         SymbolType::SYMBOL_NAME_LOWER_CASE,
         sequence_name
      );
      sequence_names_to_evaluate.emplace_back(sequence_name);
   }
   if (sequence_names.empty()) {
      for (const auto& [sequence_name, _] :
           table->schema.getColumnByType<typename SymbolType::Column>()) {
         sequence_names_to_evaluate.emplace_back(sequence_name);
      }
   }

   auto output_fields = getOutputSchema(table->schema);

   double given_min_proportion = min_proportion;

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [table,
       given_min_proportion,
       output_fields,
       partition_filters,
       sequence_names_to_evaluate,
       produced = false]() mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      EVOBENCH_SCOPE("Mutations", "producer");

      if (produced == true) {
         std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      produced = true;

      std::unordered_map<std::string, Mutations<SymbolType>::PrefilteredBitmaps>
         bitmaps_to_evaluate = preFilterBitmaps(*table, partition_filters);

      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder> output_builder;
      for (const auto& output_field : output_fields) {
         output_builder.emplace(
            output_field.name, exec_node::columnTypeToArrowType(output_field.type)
         );
      }

      for (const auto& sequence_name : sequence_names_to_evaluate) {
         const storage::column::SequenceColumnMetadata<SymbolType>* sequence_column_metadata =
            table->schema.getColumnMetadata<typename SymbolType::Column>(sequence_name).value();

         if (bitmaps_to_evaluate.contains(sequence_name)) {
            ARROW_RETURN_NOT_OK(addMutationsToOutput(
               sequence_name,
               *sequence_column_metadata,
               given_min_proportion,
               bitmaps_to_evaluate.at(sequence_name),
               output_builder
            ));
         }
      }
      // Order of result_columns is relevant as it needs to be consistent with vector in schema
      std::vector<arrow::Datum> result_columns;
      for (const auto& output_field : output_fields) {
         if (auto array_builder = output_builder.find(output_field.name);
             array_builder != output_builder.end()) {
            arrow::Datum datum;
            ARROW_ASSIGN_OR_RAISE(datum, array_builder->second.toDatum());
            result_columns.push_back(std::move(datum));
         }
      }
      ARROW_ASSIGN_OR_RAISE(
         std::optional<arrow::ExecBatch> result, arrow::ExecBatch::Make(result_columns)
      );
      return arrow::Future{result};
   };

   ARROW_ASSIGN_OR_RAISE(auto arrow_plan, arrow::acero::ExecPlan::Make());

   arrow::acero::SourceNodeOptions options{
      exec_node::columnsToArrowSchema(getOutputSchema(table->schema)),
      std::move(producer),
      arrow::Ordering::Implicit()
   };
   ARROW_ASSIGN_OR_RAISE(
      auto node, arrow::acero::MakeExecNode("source", arrow_plan.get(), {}, options)
   );

   ARROW_ASSIGN_OR_RAISE(node, addOrderingNodes(arrow_plan.get(), node, table->schema));

   ARROW_ASSIGN_OR_RAISE(node, addLimitAndOffsetNode(arrow_plan.get(), node));

   return QueryPlan::makeQueryPlan(arrow_plan, node, request_id);
}

template <typename SymbolType>
std::vector<schema::ColumnIdentifier> Mutations<SymbolType>::getOutputSchema(
   const silo::schema::TableSchema& table_schema
) const {
   using silo::schema::ColumnType;
   std::vector<schema::ColumnIdentifier> output_fields;
   if (std::ranges::find(fields, MUTATION_FIELD_NAME) != fields.end()) {
      output_fields.emplace_back(std::string(MUTATION_FIELD_NAME), ColumnType::STRING);
   }
   if (std::ranges::find(fields, MUTATION_FROM_FIELD_NAME) != fields.end()) {
      output_fields.emplace_back(std::string(MUTATION_FROM_FIELD_NAME), ColumnType::STRING);
   }
   if (std::ranges::find(fields, MUTATION_TO_FIELD_NAME) != fields.end()) {
      output_fields.emplace_back(std::string(MUTATION_TO_FIELD_NAME), ColumnType::STRING);
   }
   if (std::ranges::find(fields, SEQUENCE_FIELD_NAME) != fields.end()) {
      output_fields.emplace_back(std::string(SEQUENCE_FIELD_NAME), ColumnType::STRING);
   }
   if (std::ranges::find(fields, POSITION_FIELD_NAME) != fields.end()) {
      output_fields.emplace_back(std::string(POSITION_FIELD_NAME), ColumnType::INT32);
   }
   if (std::ranges::find(fields, PROPORTION_FIELD_NAME) != fields.end()) {
      output_fields.emplace_back(std::string(PROPORTION_FIELD_NAME), ColumnType::FLOAT);
   }
   if (std::ranges::find(fields, COVERAGE_FIELD_NAME) != fields.end()) {
      output_fields.emplace_back(std::string(COVERAGE_FIELD_NAME), ColumnType::INT32);
   }
   if (std::ranges::find(fields, COUNT_FIELD_NAME) != fields.end()) {
      output_fields.emplace_back(std::string(COUNT_FIELD_NAME), ColumnType::INT32);
   }
   return output_fields;
}

namespace {

const std::string SEQUENCE_NAMES_FIELD_NAME = "sequenceNames";
const std::string MIN_PROPORTION_FIELD_NAME = "minProportion";

}  // namespace

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Mutations<SymbolType>>& action) {
   std::vector<std::string> sequence_names;
   if (json.contains(SEQUENCE_NAMES_FIELD_NAME)) {
      CHECK_SILO_QUERY(
         json[SEQUENCE_NAMES_FIELD_NAME].is_array(),
         "Mutations action can have the field {} of type array of "
         "strings, but no other type",
         SEQUENCE_NAMES_FIELD_NAME
      );
      for (const auto& child : json[SEQUENCE_NAMES_FIELD_NAME]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field {}"
            " of Mutations action must have type "
            "array, if present. Found: {}",
            SEQUENCE_NAMES_FIELD_NAME,
            child.dump()
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   }

   CHECK_SILO_QUERY(
      json.contains(MIN_PROPORTION_FIELD_NAME) && json[MIN_PROPORTION_FIELD_NAME].is_number(),
      "Mutations action must contain the field {0}"
      " of type number with limits [0.0, "
      "1.0]. Only mutations are returned if the proportion of sequences having this mutation, "
      "is at least {0}",
      MIN_PROPORTION_FIELD_NAME
   );
   const double min_proportion = json[MIN_PROPORTION_FIELD_NAME].get<double>();
   if (min_proportion < 0 || min_proportion > 1) {
      throw BadRequest(
         "Invalid proportion: " + MIN_PROPORTION_FIELD_NAME + " must be in interval [0.0, 1.0]"
      );
   }

   std::vector<std::string_view> fields;
   if (json.contains("fields")) {
      CHECK_SILO_QUERY(
         json["fields"].is_array(),
         "The field 'fields' for a Mutations action must be an array of strings"
      );
      for (const auto& field_json : json["fields"]) {
         CHECK_SILO_QUERY(
            field_json.is_string(),
            "The field 'fields' for a Mutations action must be an array of strings"
         );
         const std::string field = field_json;
         auto it =
            std::ranges::find_if(Mutations<SymbolType>::VALID_FIELDS, [&](const auto& valid_field) {
               return valid_field == field;
            });
         CHECK_SILO_QUERY(
            it != Mutations<SymbolType>::VALID_FIELDS.end(),
            "The attribute 'fields' contains an invalid field '{}'. Valid fields are {}.",
            field,
            boost::join(
               std::vector<std::string>{
                  Mutations<SymbolType>::VALID_FIELDS.begin(),
                  Mutations<SymbolType>::VALID_FIELDS.end()
               },
               ", "
            )
         );
         fields.push_back(*it);
      }
   }

   action = std::make_unique<Mutations<SymbolType>>(
      std::move(sequence_names), min_proportion, std::move(fields)
   );
}

template class Mutations<AminoAcid>;
template class Mutations<Nucleotide>;
// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<Mutations<AminoAcid>>& action
);
// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<Mutations<Nucleotide>>& action
);

}  // namespace silo::query_engine::actions
