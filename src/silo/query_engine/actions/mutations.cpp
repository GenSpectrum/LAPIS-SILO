#include "silo/query_engine/actions/mutations.h"

#include <cmath>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string/join.hpp>
#include <nlohmann/json.hpp>

#include "evobench/count_threads.hpp"
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

template <typename SymbolType>
void Mutations<SymbolType>::addPositionToMutationCountsForMixedBitmaps(
   uint32_t position_idx,
   const PrefilteredBitmaps& bitmaps_to_evaluate,
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   for (const auto& [filter, sequence_store_partition] : bitmaps_to_evaluate.bitmaps) {
      for (const auto symbol : SymbolType::SYMBOLS) {
         const auto& current_position = sequence_store_partition.positions[position_idx];
         if (current_position.isSymbolDeleted(symbol)) {
            count_of_mutations_per_position[symbol][position_idx] += filter->cardinality();
            for (const uint32_t idx : *filter) {
               const roaring::Roaring& n_bitmap =
                  sequence_store_partition.missing_symbol_bitmaps[idx];
               if (n_bitmap.contains(position_idx)) {
                  count_of_mutations_per_position[symbol][position_idx] -= 1;
               }
            }
            continue;
         }
         const uint32_t symbol_count =
            current_position.isSymbolFlipped(symbol)
               ? filter->andnot_cardinality(*current_position.getBitmap(symbol))
               : filter->and_cardinality(*current_position.getBitmap(symbol));

         count_of_mutations_per_position[symbol][position_idx] += symbol_count;

         const auto deleted_symbol = current_position.getDeletedSymbol();
         if (deleted_symbol.has_value() && symbol != *deleted_symbol) {
            count_of_mutations_per_position[*deleted_symbol][position_idx] -= symbol_count;
         }
      }
   }
}

template <typename SymbolType>
void Mutations<SymbolType>::addPositionToMutationCountsForFullBitmaps(
   uint32_t position_idx,
   const PrefilteredBitmaps& bitmaps_to_evaluate,
   SymbolMap<SymbolType, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   // For these partitions, we have full bitmaps. Do not need to bother with AND
   // cardinality
   for (const auto& [_, sequence_store_partition] : bitmaps_to_evaluate.full_bitmaps) {
      for (const auto symbol : SymbolType::SYMBOLS) {
         const auto& current_position = sequence_store_partition.positions[position_idx];
         if (current_position.isSymbolDeleted(symbol)) {
            count_of_mutations_per_position[symbol][position_idx] +=
               sequence_store_partition.sequence_count;
            for (const roaring::Roaring& n_bitmap :
                 sequence_store_partition.missing_symbol_bitmaps) {
               if (n_bitmap.contains(position_idx)) {
                  count_of_mutations_per_position[symbol][position_idx] -= 1;
               }
            }
            continue;
         }
         const uint32_t symbol_count = current_position.isSymbolFlipped(symbol)
                                          ? sequence_store_partition.sequence_count -
                                               current_position.getBitmap(symbol)->cardinality()
                                          : current_position.getBitmap(symbol)->cardinality();

         count_of_mutations_per_position[symbol][position_idx] += symbol_count;

         const auto deleted_symbol = current_position.getDeletedSymbol();
         if (deleted_symbol.has_value() && symbol != *deleted_symbol) {
            count_of_mutations_per_position[*deleted_symbol][position_idx] -=
               sequence_store_partition.positions[position_idx].getBitmap(symbol)->cardinality();
         }
      }
   }
}

template <typename SymbolType>
SymbolMap<SymbolType, std::vector<uint32_t>> Mutations<SymbolType>::calculateMutationsPerPosition(
   const storage::column::SequenceColumnMetadata<SymbolType>& metadata,
   const PrefilteredBitmaps& bitmap_filter
) {
   EVOBENCH_SCOPE("Mutations", "calculateMutationsPerPosition");
   const size_t sequence_length = metadata.reference_sequence.size();

   SymbolMap<SymbolType, std::vector<uint32_t>> mutation_counts_per_position;
   for (const auto symbol : SymbolType::SYMBOLS) {
      mutation_counts_per_position[symbol].resize(sequence_length);
   }
   static constexpr int POSITIONS_PER_PROCESS = 300;
   // XX POSITIONS_PER_PROCESS
   for (uint32_t pos = 0; pos < sequence_length; pos++) {
      addPositionToMutationCountsForMixedBitmaps(
         pos, bitmap_filter, mutation_counts_per_position
         );
      addPositionToMutationCountsForFullBitmaps(
         pos, bitmap_filter, mutation_counts_per_position
         );
   }
   SPDLOG_INFO("thread count = {}", evobench::count_threads());
   return mutation_counts_per_position;
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
   const config::QueryOptions& query_options
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

   return QueryPlan::makeQueryPlan(arrow_plan, node);
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
