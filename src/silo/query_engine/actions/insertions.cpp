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

template <typename Symbol>
InsertionAggregation<Symbol>::InsertionAggregation(
   std::string column,
   std::vector<std::string>&& sequence_names
)
    : column_name(std::move(column)),
      sequence_names(std::move(sequence_names)) {}

template <typename Symbol>
void InsertionAggregation<Symbol>::validateOrderByFields(const Database& /*database*/) const {
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

template <>
std::unordered_map<std::string, InsertionAggregation<AA_SYMBOL>::PrefilteredBitmaps>
InsertionAggregation<AA_SYMBOL>::validateFieldsAndPreFilterBitmaps(
   const Database& database,
   std::vector<OperatorResult>& bitmap_filter
) const {
   CHECK_SILO_QUERY(
      database.columns.aa_insertion_columns.contains(column_name),
      "The column " + column_name + " does not exist."
   )
   std::unordered_map<std::string, PrefilteredBitmaps> bitmaps_to_evaluate;
   for (size_t i = 0; i < database.partitions.size(); ++i) {
      const DatabasePartition& database_partition = database.partitions.at(i);
      const auto& insertion_indexes =
         database_partition.columns.aa_insertion_columns.at(column_name).getInsertionIndexes();
      OperatorResult& filter = bitmap_filter[i];

      for (const auto& sequence_name : sequence_names) {
         CHECK_SILO_QUERY(
            insertion_indexes.contains(sequence_name),
            "The column '" + column_name + "' does not contain the sequence '" + sequence_name + "'"
         )
      }

      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == database_partition.sequence_count) {
         for (const auto& [sequence_name, sequence_index] : insertion_indexes) {
            if(sequence_names.empty() ||
                std::find(sequence_names.begin(), sequence_names.end(), sequence_name) != sequence_names.end()){
               bitmaps_to_evaluate[sequence_name].full_bitmaps.emplace_back(filter, sequence_index);
            }
         }
      } else {
         if (filter.isMutable()) {
            filter->runOptimize();
         }
         for (const auto& [sequence_name, sequence_index] : insertion_indexes) {
            if(sequence_names.empty() ||
                std::find(sequence_names.begin(), sequence_names.end(), sequence_name) != sequence_names.end()){
               bitmaps_to_evaluate[sequence_name].bitmaps.emplace_back(filter, sequence_index);
            }
         }
      }
   }
   return bitmaps_to_evaluate;
}

template <>
std::unordered_map<std::string, InsertionAggregation<NUCLEOTIDE_SYMBOL>::PrefilteredBitmaps>
InsertionAggregation<NUCLEOTIDE_SYMBOL>::validateFieldsAndPreFilterBitmaps(
   const Database& database,
   std::vector<OperatorResult>& bitmap_filter
) const {
   CHECK_SILO_QUERY(
      database.columns.nuc_insertion_columns.contains(column_name),
      "The column '" + column_name + "' does not exist."
   )

   std::unordered_map<std::string, PrefilteredBitmaps> bitmaps_to_evaluate;
   for (size_t i = 0; i < database.partitions.size(); ++i) {
      const DatabasePartition& database_partition = database.partitions.at(i);
      const auto& insertion_indexes =
         database_partition.columns.nuc_insertion_columns.at(column_name).getInsertionIndexes();
      OperatorResult& filter = bitmap_filter[i];

      for (const auto& sequence_name : sequence_names) {
         CHECK_SILO_QUERY(
            insertion_indexes.contains(sequence_name),
            "The column '" + column_name + "' does not contain the sequence '" + sequence_name + "'"
         )
      }

      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == database_partition.sequence_count) {
         for (const auto& [sequence_name, sequence_index] : insertion_indexes) {
            if(sequence_names.empty() ||
                std::find(sequence_names.begin(), sequence_names.end(), sequence_name) != sequence_names.end()){
               bitmaps_to_evaluate[sequence_name].full_bitmaps.emplace_back(filter, sequence_index);
            }
         }
      } else {
         if (filter.isMutable()) {
            filter->runOptimize();
         }
         for (const auto& [sequence_name, sequence_index] : insertion_indexes) {
            if(sequence_names.empty() ||
                std::find(sequence_names.begin(), sequence_names.end(), sequence_name) != sequence_names.end()){
               bitmaps_to_evaluate[sequence_name].bitmaps.emplace_back(filter, sequence_index);
            }
         }
      }
   }
   return bitmaps_to_evaluate;
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

template <typename Symbol>
void InsertionAggregation<Symbol>::addAggregatedInsertionsToInsertionCounts(
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
         {std::string(POSITION_FIELD_NAME),
          static_cast<int32_t>(position_and_insertion.position) + 1},
         {std::string(SEQUENCE_FIELD_NAME), sequence_name},
         {std::string(INSERTION_FIELD_NAME), std::string(position_and_insertion.insertion_value)},
         {std::string(COUNT_FIELD_NAME), static_cast<int32_t>(count)}};
      output.push_back({fields});
   }
}

template <typename Symbol>
QueryResult InsertionAggregation<Symbol>::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   using storage::column::insertion::InsertionIndex;

   std::unordered_map<std::string, InsertionAggregation<Symbol>::PrefilteredBitmaps>
      bitmaps_to_evaluate = validateFieldsAndPreFilterBitmaps(database, bitmap_filter);

   std::vector<QueryResultEntry> insertion_counts;
   for (const auto& [sequence_name, prefiltered_bitmaps] : bitmaps_to_evaluate) {
      addAggregatedInsertionsToInsertionCounts(
         insertion_counts, sequence_name, prefiltered_bitmaps
      );
   }
   return {insertion_counts};
}

template <typename Symbol>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<InsertionAggregation<Symbol>>& action) {
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
      json.contains("column") && json["column"].is_string(),
      "Insertions must have the field 'column' of type string"
   )
   const std::string column = json["column"].get<std::string>();

   action = std::make_unique<InsertionAggregation<Symbol>>(column, std::move(sequence_names));
}

template void from_json<AA_SYMBOL>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<AA_SYMBOL>>& action
);

template void from_json<NUCLEOTIDE_SYMBOL>(
   const nlohmann::json& json,
   std::unique_ptr<InsertionAggregation<NUCLEOTIDE_SYMBOL>>& action
);

template class InsertionAggregation<NUCLEOTIDE_SYMBOL>;
template class InsertionAggregation<AA_SYMBOL>;

}  // namespace silo::query_engine::actions
