#include "silo/query_engine/actions/insertions.h"

#include <cmath>
#include <deque>
#include <map>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <boost/functional/hash.hpp>
#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/column/insertion_column.h"
#include "silo/storage/column/insertion_index.h"

using silo::query_engine::OperatorResult;

namespace silo::query_engine::actions {

template <typename SymbolType>
InsertionAggregation<SymbolType>::InsertionAggregation(
   std::vector<std::string>&& column_names,
   std::vector<std::string>&& sequence_names
)
    : column_names(std::move(column_names)),
      sequence_names(std::move(sequence_names)) {}

template <typename SymbolType>
void InsertionAggregation<SymbolType>::validateOrderByFields(const Database& /*database*/) const {
   const std::vector<std::string> result_field_names{
      {std::string{POSITION_FIELD_NAME},
       std::string{INSERTION_FIELD_NAME},
       std::string{SEQUENCE_FIELD_NAME},
       std::string{COUNT_FIELD_NAME}}};

   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         std::any_of(
            result_field_names.begin(),
            result_field_names.end(),
            [&](const std::string& result_field) { return result_field == field.name; }
         ),
         "OrderByField " + field.name + " is not contained in the result of this operation."
      )
   }
}

template <typename SymbolType>
void validateColumnNames(
   const storage::ColumnPartitionGroup& column_group,
   const std::vector<std::string>& column_names
) {
   for (std::string column_name : column_names) {
      CHECK_SILO_QUERY(
         column_group.getInsertionColumns<SymbolType>().contains(column_name),
         "The column '" + column_name + "' does not exist."
      )
   }
}

template <typename SymbolType>
void InsertionAggregation<SymbolType>::addAllColumnIndexesToPreFilteredBitmaps(
   const storage::column::InsertionColumnPartition<SymbolType>& column,
   const OperatorResult& filter,
   std::unordered_map<std::string, InsertionAggregation<SymbolType>::PrefilteredBitmaps>&
      bitmaps_to_evaluate
) const {
   for (const auto& [sequence_name, sequence_index] : column.getInsertionIndexes()) {
      if(sequence_names.empty() ||
          std::find(sequence_names.begin(), sequence_names.end(), sequence_name) != sequence_names.end()){
         bitmaps_to_evaluate[sequence_name].bitmaps.emplace_back(filter, sequence_index);
      }
   }
}

template <typename SymbolType>
std::unordered_map<std::string, typename InsertionAggregation<SymbolType>::PrefilteredBitmaps>
InsertionAggregation<SymbolType>::validateFieldsAndPreFilterBitmaps(
   const Database& database,
   std::vector<OperatorResult>& bitmap_filter
) const {
   for (const std::string& column_name : column_names) {
      CHECK_SILO_QUERY(
         database.columns.getInsertionColumns<SymbolType>().contains(column_name),
         "The database does not contain the " + std::string(SymbolType::SYMBOL_NAME) + " column '" +
            column_name + "'"
      );
   }
   std::vector<std::string> all_sequence_names = database.getSequenceNames<SymbolType>();
   for (const std::string& sequence_name : sequence_names) {
      CHECK_SILO_QUERY(
         std::find(all_sequence_names.begin(), all_sequence_names.end(), sequence_name) !=
            all_sequence_names.end(),
         "The database does not contain the " + std::string(SymbolType::SYMBOL_NAME) +
            " sequence '" + sequence_name + "'"
      );
   }

   std::unordered_map<std::string, PrefilteredBitmaps> pre_filtered_bitmaps;
   for (size_t i = 0; i < database.partitions.size(); ++i) {
      const DatabasePartition& database_partition = database.partitions.at(i);

      validateColumnNames<SymbolType>(database_partition.columns, column_names);

      for (auto& [column_name, insertion_column] :
           database_partition.columns.getInsertionColumns<SymbolType>()) {
         if(column_names.empty() ||
             std::find(column_names.begin(), column_names.end(), column_name) != column_names.end()){
            OperatorResult& filter = bitmap_filter[i];
            const size_t cardinality = filter->cardinality();
            if (cardinality == 0) {
               continue;
            }
            if (cardinality == database_partition.sequence_count) {
               addAllColumnIndexesToPreFilteredBitmaps(
                  insertion_column, filter, pre_filtered_bitmaps
               );
            } else {
               if (filter.isMutable()) {
                  filter->runOptimize();
               }
               addAllColumnIndexesToPreFilteredBitmaps(
                  insertion_column, filter, pre_filtered_bitmaps
               );
            }
         }
      }
   }
   return pre_filtered_bitmaps;
}

struct PositionAndInsertion {
   uint32_t position;
   std::string_view insertion_value;

   bool operator==(const PositionAndInsertion& other) const {
      return position == other.position && insertion_value == other.insertion_value;
   }
};
}  // namespace silo::query_engine::actions

using silo::query_engine::actions::PositionAndInsertion;

template <>
struct std::hash<PositionAndInsertion> {
   std::size_t operator()(const PositionAndInsertion& position_and_insertion) const noexcept {
      std::size_t seed = 0;
      boost::hash_combine(seed, position_and_insertion.position);
      boost::hash_combine(seed, position_and_insertion.insertion_value);
      return seed;
   }
};

namespace silo::query_engine::actions {

template <typename SymbolType>
void InsertionAggregation<SymbolType>::addAggregatedInsertionsToInsertionCounts(
   std::vector<QueryResultEntry>& output,
   const std::string& sequence_name,
   const PrefilteredBitmaps& prefiltered_bitmaps
) const {
   std::unordered_map<PositionAndInsertion, uint32_t> all_insertions;
   for (const auto& [_, insertion_index] : prefiltered_bitmaps.full_bitmaps) {
      for (const auto& [position, insertions_at_position] :
           insertion_index.getInsertionPositions()) {
         for (const auto& insertion : insertions_at_position.insertions) {
            all_insertions[PositionAndInsertion{position, insertion.value}] +=
               insertion.sequence_ids.cardinality();
         }
      }
   }
   for (const auto& [bitmap_filter, insertion_index] : prefiltered_bitmaps.bitmaps) {
      for (const auto& [position, insertions_at_position] :
           insertion_index.getInsertionPositions()) {
         for (const auto& insertion : insertions_at_position.insertions) {
            const uint32_t count = insertion.sequence_ids.and_cardinality(*bitmap_filter);
            if (count > 0) {
               all_insertions[PositionAndInsertion{position, insertion.value}] += count;
            }
         }
      }
   }
   for (const auto& [position_and_insertion, count] : all_insertions) {
      const std::map<std::string, std::optional<std::variant<std::string, int32_t, double>>> fields{
         {std::string(POSITION_FIELD_NAME), static_cast<int32_t>(position_and_insertion.position)},
         {std::string(SEQUENCE_FIELD_NAME), sequence_name},
         {std::string(INSERTION_FIELD_NAME), std::string(position_and_insertion.insertion_value)},
         {std::string(COUNT_FIELD_NAME), static_cast<int32_t>(count)}};
      output.push_back({fields});
   }
}

template <typename SymbolType>
QueryResult InsertionAggregation<SymbolType>::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   using storage::column::insertion::InsertionIndex;

   std::unordered_map<std::string, InsertionAggregation<SymbolType>::PrefilteredBitmaps>
      bitmaps_to_evaluate = validateFieldsAndPreFilterBitmaps(database, bitmap_filter);

   std::vector<QueryResultEntry> insertion_counts;
   for (const auto& [sequence_name, prefiltered_bitmaps] : bitmaps_to_evaluate) {
      addAggregatedInsertionsToInsertionCounts(
         insertion_counts, sequence_name, prefiltered_bitmaps
      );
   }
   return {insertion_counts};
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<SymbolType>>& action
) {
   CHECK_SILO_QUERY(
      !json.contains("sequenceName") ||
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "Insertions action can have the field sequenceName of type string or an array of "
      "strings, but no other type"
   )
   std::vector<std::string> sequence_names;
   if (json.contains("sequenceName") && json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field sequenceName of the Insertions action must have type string or an "
            "array, if present. Found:" +
               child.dump()
         )
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else if (json.contains("sequenceName") && json["sequenceName"].is_string()) {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }

   CHECK_SILO_QUERY(
      !json.contains("column") || (json["column"].is_string() || json["column"].is_array()),
      "Insertions action can have the field column of type string or an array of "
      "strings, but no other type"
   )
   std::vector<std::string> column_names;
   if (json.contains("column") && json.at("column").is_array()) {
      for (const auto& child : json["column"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field column of the Insertions action must have type string or an "
            "array, if present. Found:" +
               child.dump()
         )
         column_names.emplace_back(child.get<std::string>());
      }
   } else if (json.contains("column") && json["column"].is_string()) {
      column_names.emplace_back(json["column"].get<std::string>());
   }

   action = std::make_unique<InsertionAggregation<SymbolType>>(
      std::move(column_names), std::move(sequence_names)
   );
}

template void from_json<AminoAcid>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<AminoAcid>>& action
);

template void from_json<Nucleotide>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<Nucleotide>>& action
);

template class InsertionAggregation<Nucleotide>;
template class InsertionAggregation<AminoAcid>;

}  // namespace silo::query_engine::actions
