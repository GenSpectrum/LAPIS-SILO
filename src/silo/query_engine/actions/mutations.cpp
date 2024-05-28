#include "silo/query_engine/actions/mutations.h"

#include <cmath>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <fmt/format.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <nlohmann/json.hpp>

#include "silo/common/aa_symbols.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/symbol_map.h"
#include "silo/database.h"
#include "silo/query_engine/actions/action.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/sequence_store.h"

using silo::query_engine::OperatorResult;

namespace silo::query_engine::actions {

template <typename SymbolType>
Mutations<SymbolType>::Mutations(std::vector<std::string>&& sequence_names, double min_proportion)
    : sequence_names(std::move(sequence_names)),
      min_proportion(min_proportion) {}

template <typename SymbolType>
std::unordered_map<std::string, typename Mutations<SymbolType>::PrefilteredBitmaps> Mutations<
   SymbolType>::
   preFilterBitmaps(const silo::Database& database, std::vector<OperatorResult>& bitmap_filter) {
   std::unordered_map<std::string, PrefilteredBitmaps> bitmaps_to_evaluate;
   for (size_t i = 0; i < database.partitions.size(); ++i) {
      const DatabasePartition& database_partition = database.partitions.at(i);
      OperatorResult& filter = bitmap_filter[i];
      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == database_partition.sequence_count) {
         for (const auto& [sequence_name, sequence_store] :
              database_partition.getSequenceStores<SymbolType>()) {
            bitmaps_to_evaluate[sequence_name].full_bitmaps.emplace_back(filter, sequence_store);
         }
      } else {
         if (filter.isMutable()) {
            filter->runOptimize();
         }
         for (const auto& [sequence_name, sequence_store] :
              database_partition.getSequenceStores<SymbolType>()) {
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
   for (const auto& [filter, sequence_store_partition] : bitmaps_to_evaluate.full_bitmaps) {
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
   const SequenceStore<SymbolType>& sequence_store,
   const PrefilteredBitmaps& bitmap_filter
) {
   const size_t sequence_length = sequence_store.reference_sequence.size();

   SymbolMap<SymbolType, std::vector<uint32_t>> mutation_counts_per_position;
   for (const auto symbol : SymbolType::SYMBOLS) {
      mutation_counts_per_position[symbol].resize(sequence_length);
   }
   static constexpr int POSITIONS_PER_PROCESS = 300;
   tbb::parallel_for(
      tbb::blocked_range<uint32_t>(0, sequence_length, /*grain_size=*/POSITIONS_PER_PROCESS),
      [&](const auto& local) {
         for (uint32_t pos = local.begin(); pos != local.end(); ++pos) {
            addPositionToMutationCountsForMixedBitmaps(
               pos, bitmap_filter, mutation_counts_per_position
            );
            addPositionToMutationCountsForFullBitmaps(
               pos, bitmap_filter, mutation_counts_per_position
            );
         }
      }
   );
   return mutation_counts_per_position;
}

template <typename SymbolType>
void Mutations<SymbolType>::validateOrderByFields(const Database& /*database*/) const {
   const std::vector<std::string> result_field_names{
      {MUTATION_FIELD_NAME,
       MUTATION_FROM_FIELD_NAME,
       MUTATION_TO_FIELD_NAME,
       POSITION_FIELD_NAME,
       PROPORTION_FIELD_NAME,
       SEQUENCE_FIELD_NAME,
       COUNT_FIELD_NAME}
   };

   for (const OrderByField& field : order_by_fields) {
      CHECK_SILO_QUERY(
         std::any_of(
            result_field_names.begin(),
            result_field_names.end(),
            [&](const std::string& result_field) { return result_field == field.name; }
         ),
         fmt::format(
            "OrderByField {} is not contained in the result of this operation. "
            "Allowed values are {}.",
            field.name,
            fmt::join(result_field_names, ", ")
         )
      );
   }
}

template <typename SymbolType>
void Mutations<SymbolType>::addMutationsToOutput(
   const std::string& sequence_name,
   const SequenceStore<SymbolType>& sequence_store,
   const PrefilteredBitmaps& bitmap_filter,
   std::vector<QueryResultEntry>& output
) const {
   const size_t sequence_length = sequence_store.reference_sequence.size();

   const SymbolMap<SymbolType, std::vector<uint32_t>> count_of_mutations_per_position =
      calculateMutationsPerPosition(sequence_store, bitmap_filter);

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
         sequence_store.reference_sequence.at(pos);

      for (const auto symbol : SymbolType::VALID_MUTATION_SYMBOLS) {
         if (symbol_in_reference_genome != symbol) {
            const uint32_t count = count_of_mutations_per_position.at(symbol)[pos];
            if (count > threshold_count) {
               const double proportion = static_cast<double>(count) / static_cast<double>(total);
               const std::map<std::string, common::JsonValueType> fields{
                  {
                     MUTATION_FIELD_NAME,
                     fmt::format(
                        "{}{}{}",
                        SymbolType::symbolToChar(symbol_in_reference_genome),
                        pos + 1,
                        SymbolType::symbolToChar(symbol)
                     ),
                  },
                  {
                     MUTATION_FROM_FIELD_NAME,
                     std::string(1, SymbolType::symbolToChar(symbol_in_reference_genome)),
                  },
                  {MUTATION_TO_FIELD_NAME, std::string(1, SymbolType::symbolToChar(symbol))},
                  {POSITION_FIELD_NAME, static_cast<int32_t>(pos + 1)},
                  {SEQUENCE_FIELD_NAME, sequence_name},
                  {PROPORTION_FIELD_NAME, proportion},
                  {COUNT_FIELD_NAME, static_cast<int32_t>(count)}
               };
               output.push_back({fields});
            }
         }
      }
   }
}

template <typename SymbolType>
QueryResult Mutations<SymbolType>::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   std::vector<std::string> sequence_names_to_evaluate;
   for (const auto& sequence_name : sequence_names) {
      CHECK_SILO_QUERY(
         database.getSequenceStores<SymbolType>().contains(sequence_name),
         "Database does not contain the " + std::string(SymbolType::SYMBOL_NAME_LOWER_CASE) +
            " sequence with name: '" + sequence_name + "'"
      );
      sequence_names_to_evaluate.emplace_back(sequence_name);
   }
   if (sequence_names.empty()) {
      for (const auto& [sequence_name, _] : database.getSequenceStores<SymbolType>()) {
         sequence_names_to_evaluate.emplace_back(sequence_name);
      }
   }

   std::unordered_map<std::string, Mutations<SymbolType>::PrefilteredBitmaps> bitmaps_to_evaluate =
      preFilterBitmaps(database, bitmap_filter);

   std::vector<QueryResultEntry> mutation_proportions;
   for (const auto& sequence_name : sequence_names_to_evaluate) {
      const SequenceStore<SymbolType>& sequence_store =
         database.getSequenceStores<SymbolType>().at(sequence_name);

      if (bitmaps_to_evaluate.contains(sequence_name)) {
         addMutationsToOutput(
            sequence_name,
            sequence_store,
            bitmaps_to_evaluate.at(sequence_name),
            mutation_proportions
         );
      }
   }
   return {mutation_proportions};
}

template <typename SymbolType>
// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<Mutations<SymbolType>>& action) {
   CHECK_SILO_QUERY(
      !json.contains("sequenceName") ||
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "Mutations action can have the field sequenceName of type string or an array of "
      "strings, but no other type"
   );
   std::vector<std::string> sequence_names;
   if (json.contains("sequenceName") && json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field sequenceName of Mutations action must have type string or an "
            "array, if present. Found:" +
               child.dump()
         );
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else if (json.contains("sequenceName") && json["sequenceName"].is_string()) {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }

   CHECK_SILO_QUERY(
      json.contains("minProportion") && json["minProportion"].is_number(),
      "Mutations action must contain the field minProportion of type number with limits [0.0, "
      "1.0]. Only mutations are returned if the proportion of sequences having this mutation, "
      "is "
      "at least minProportion"
   );
   const double min_proportion = json["minProportion"].get<double>();
   if (min_proportion < 0 || min_proportion > 1) {
      throw QueryParseException("Invalid proportion: minProportion must be in interval [0.0, 1.0]");
   }

   action = std::make_unique<Mutations<SymbolType>>(std::move(sequence_names), min_proportion);
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
