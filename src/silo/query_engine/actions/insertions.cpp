#include "silo/query_engine/actions/insertions.h"

#include <algorithm>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <arrow/acero/options.h>
#include <arrow/compute/exec.h>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <boost/container_hash/hash.hpp>
#include <nlohmann/json.hpp>

#include "evobench/evobench.hpp"
#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/bad_request.h"
#include "silo/query_engine/copy_on_write_bitmap.h"
#include "silo/query_engine/exec_node/arrow_util.h"
#include "silo/storage/column/insertion_index.h"
#include "silo/storage/table_partition.h"

using silo::query_engine::CopyOnWriteBitmap;
using silo::storage::insertion::InsertionIndex;

namespace silo::query_engine::actions {

template <typename SymbolType>
InsertionAggregation<SymbolType>::InsertionAggregation(std::vector<std::string>&& sequence_names)
    : sequence_names(std::move(sequence_names)) {}

template <typename SymbolType>
void InsertionAggregation<
   SymbolType>::validateOrderByFields(const schema::TableSchema& /*table_schema*/) const {
   const std::vector<std::string> result_field_names{
      {std::string{POSITION_FIELD_NAME},
       std::string{INSERTION_FIELD_NAME},
       std::string{SEQUENCE_FIELD_NAME},
       std::string{COUNT_FIELD_NAME},
       std::string{INSERTED_SYMBOLS_FIELD_NAME}}
   };

   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         std::ranges::any_of(
            result_field_names,
            [&](const std::string& result_field) { return result_field == field.name; }
         ),
         "OrderByField {} is not contained in the result of this operation. "
         "Allowed values are {}.",
         field.name,
         fmt::join(result_field_names, ", ")
      );
   }
}

namespace {
template <typename SymbolType>
void validateSequenceNames(
   std::shared_ptr<const storage::Table> table,
   const std::vector<std::string>& sequence_names
) {
   for (const std::string& sequence_name : sequence_names) {
      auto column = table->schema.getColumn(sequence_name);
      CHECK_SILO_QUERY(
         column.has_value() && column.value().type == SymbolType::COLUMN_TYPE,
         "The database does not contain the {} sequence '{}'",
         SymbolType::SYMBOL_NAME,
         sequence_name
      );
   }
}
}  // namespace

template <typename SymbolType>
std::unordered_map<std::string, typename InsertionAggregation<SymbolType>::PrefilteredBitmaps>
InsertionAggregation<SymbolType>::preFilterBitmaps(
   std::shared_ptr<const storage::Table> table,
   const std::vector<std::string>& sequence_names,
   std::vector<CopyOnWriteBitmap>& bitmap_filter
) {
   std::unordered_map<std::string, PrefilteredBitmaps> pre_filtered_bitmaps;
   for (size_t i = 0; i < table->getNumberOfPartitions(); ++i) {
      const storage::TablePartition& table_partition = table->getPartition(i);

      for (auto& [sequence_name, sequence_column] :
           table_partition.columns.getColumns<typename SymbolType::Column>()) {
         if (sequence_names.empty() ||
             std::ranges::find(sequence_names, sequence_name) != sequence_names.end()) {
            CopyOnWriteBitmap& filter = bitmap_filter[i];
            const size_t cardinality = filter->cardinality();
            if (cardinality == 0) {
               continue;
            }
            if (cardinality == table_partition.sequence_count) {
               pre_filtered_bitmaps[sequence_name].full_bitmaps.emplace_back(
                  cardinality, sequence_column.insertion_index
               );
            } else {
               if (filter.isMutable()) {
                  filter->runOptimize();
               }
               pre_filtered_bitmaps[sequence_name].bitmaps.emplace_back(
                  filter, sequence_column.insertion_index
               );
            }
         }
      }
   }
   return pre_filtered_bitmaps;
}

struct PositionAndInsertion {
   uint32_t position_idx;
   std::string_view insertion_value;

   bool operator==(const PositionAndInsertion& other) const {
      return position_idx == other.position_idx && insertion_value == other.insertion_value;
   }
};
}  // namespace silo::query_engine::actions

using silo::query_engine::actions::PositionAndInsertion;

template <>
struct std::hash<PositionAndInsertion> {
   std::size_t operator()(const PositionAndInsertion& position_and_insertion) const noexcept {
      std::size_t seed = 0;
      boost::hash_combine(seed, position_and_insertion.position_idx);
      boost::hash_combine(seed, position_and_insertion.insertion_value);
      return seed;
   }
};

namespace silo::query_engine::actions {

template <typename SymbolType>
arrow::Status InsertionAggregation<SymbolType>::addAggregatedInsertionsToInsertionCounts(
   const std::string& sequence_name,
   bool show_sequence_in_response,
   const PrefilteredBitmaps& prefiltered_bitmaps,
   std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder>& output_builder
) {
   std::unordered_map<PositionAndInsertion, uint32_t> all_insertions;
   for (const auto& [_, insertion_index] : prefiltered_bitmaps.full_bitmaps) {
      for (const auto& [position, insertions_at_position] :
           insertion_index.getInsertionPositions()) {
         for (const auto& insertion : insertions_at_position.insertions) {
            all_insertions[PositionAndInsertion{position, insertion.value}] +=
               insertion.row_ids.cardinality();
         }
      }
   }
   for (const auto& [bitmap_filter, insertion_index] : prefiltered_bitmaps.bitmaps) {
      for (const auto& [position, insertions_at_position] :
           insertion_index.getInsertionPositions()) {
         for (const auto& insertion : insertions_at_position.insertions) {
            const uint32_t count = insertion.row_ids.and_cardinality(*bitmap_filter);
            if (count > 0) {
               all_insertions[PositionAndInsertion{position, insertion.value}] += count;
            }
         }
      }
   }
   const std::string sequence_in_response = show_sequence_in_response ? sequence_name + ":" : "";
   for (const auto& [position_and_insertion, count] : all_insertions) {
      if (auto builder = output_builder.find(POSITION_FIELD_NAME);
          builder != output_builder.end()) {
         ARROW_RETURN_NOT_OK(
            builder->second.insert(static_cast<int32_t>(position_and_insertion.position_idx))
         );
      }
      if (auto builder = output_builder.find(INSERTED_SYMBOLS_FIELD_NAME);
          builder != output_builder.end()) {
         ARROW_RETURN_NOT_OK(
            builder->second.insert(std::string(position_and_insertion.insertion_value))
         );
      }
      if (auto builder = output_builder.find(SEQUENCE_FIELD_NAME);
          builder != output_builder.end()) {
         ARROW_RETURN_NOT_OK(builder->second.insert(sequence_name));
      }
      if (auto builder = output_builder.find(INSERTION_FIELD_NAME);
          builder != output_builder.end()) {
         ARROW_RETURN_NOT_OK(builder->second.insert({fmt::format(
            "ins_{}{}:{}",
            sequence_in_response,
            position_and_insertion.position_idx,
            position_and_insertion.insertion_value
         )}));
      }
      if (auto builder = output_builder.find(COUNT_FIELD_NAME); builder != output_builder.end()) {
         ARROW_RETURN_NOT_OK(builder->second.insert({static_cast<int32_t>(count)}));
      }
   }
   return arrow::Status::OK();
}

template <typename SymbolType>
arrow::Result<QueryPlan> InsertionAggregation<SymbolType>::toQueryPlanImpl(
   std::shared_ptr<const storage::Table> table,
   std::vector<CopyOnWriteBitmap> partition_filters,
   const config::QueryOptions& query_options
) const {
   EVOBENCH_SCOPE("InsertionAggregation", "toQueryPlanImpl");
   validateSequenceNames<SymbolType>(table, sequence_names);
   auto sequence_names_to_evaluate = sequence_names;

   auto output_fields = getOutputSchema(table->schema);

   std::function<arrow::Future<std::optional<arrow::ExecBatch>>()> producer =
      [table, output_fields, partition_filters, sequence_names_to_evaluate, produced = false](
      ) mutable -> arrow::Future<std::optional<arrow::ExecBatch>> {
      EVOBENCH_SCOPE("InsertionAggregation", "producer");
      if (produced == true) {
         std::optional<arrow::ExecBatch> result = std::nullopt;
         return arrow::Future{result};
      }
      produced = true;

      std::unordered_map<std::string_view, exec_node::JsonValueTypeArrayBuilder> output_builder;
      for (const auto& output_field : output_fields) {
         output_builder.emplace(
            output_field.name, exec_node::columnTypeToArrowType(output_field.type)
         );
      }

      const auto bitmaps_to_evaluate =
         preFilterBitmaps(table, sequence_names_to_evaluate, partition_filters);
      for (const auto& [sequence_name, prefiltered_bitmaps] : bitmaps_to_evaluate) {
         const auto default_sequence_name = table->schema.getDefaultSequenceName<SymbolType>();
         const bool omit_sequence_in_response =
            default_sequence_name.has_value() &&
            (default_sequence_name.value().name == sequence_name);
         ARROW_RETURN_NOT_OK(addAggregatedInsertionsToInsertionCounts(
            sequence_name, !omit_sequence_in_response, prefiltered_bitmaps, output_builder
         ));
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
std::vector<schema::ColumnIdentifier> InsertionAggregation<SymbolType>::getOutputSchema(
   const silo::schema::TableSchema& table_schema
) const {
   std::vector<schema::ColumnIdentifier> fields;
   fields.emplace_back(std::string(POSITION_FIELD_NAME), schema::ColumnType::INT32);
   fields.emplace_back(std::string(INSERTED_SYMBOLS_FIELD_NAME), schema::ColumnType::STRING);
   fields.emplace_back(std::string(SEQUENCE_FIELD_NAME), schema::ColumnType::STRING);
   fields.emplace_back(std::string(INSERTION_FIELD_NAME), schema::ColumnType::STRING);
   fields.emplace_back(std::string(COUNT_FIELD_NAME), schema::ColumnType::INT32);
   return fields;
}

namespace {

const std::string SEQUENCE_NAMES_FIELD_NAME = "sequenceNames";

}  // namespace

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<SymbolType>>& action
) {
   std::vector<std::string> sequence_names;
   if (json.contains(SEQUENCE_NAMES_FIELD_NAME)) {
      CHECK_SILO_QUERY(
         json[SEQUENCE_NAMES_FIELD_NAME].is_array(),
         "The field '{}' of the insertions action must be of type string or array, was {}",
         SEQUENCE_NAMES_FIELD_NAME,
         std::string(json[SEQUENCE_NAMES_FIELD_NAME].type_name())
      );
      for (const auto& child : json[SEQUENCE_NAMES_FIELD_NAME]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field {} of the Insertions action must have type string or an "
            "array, if present. Found: {}",
            SEQUENCE_NAMES_FIELD_NAME,
            child.dump()
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   }

   action = std::make_unique<InsertionAggregation<SymbolType>>(std::move(sequence_names));
}

// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<AminoAcid>>& action
);

// NOLINTNEXTLINE(readability-identifier-naming)
template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<Nucleotide>>& action
);

template class InsertionAggregation<Nucleotide>;
template class InsertionAggregation<AminoAcid>;

}  // namespace silo::query_engine::actions
