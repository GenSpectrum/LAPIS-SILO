#include "silo/query_engine/actions/aa_mutations.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
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

AAMutations::AAMutations(std::vector<std::string>&& aa_sequence_names, double min_proportion)
    : aa_sequence_names(std::move(aa_sequence_names)),
      min_proportion(min_proportion) {}

std::unordered_map<std::string, AAMutations::PrefilteredBitmaps> AAMutations::preFilterBitmaps(
   const silo::Database& database,
   std::vector<OperatorResult>& bitmap_filter
) {
   std::unordered_map<std::string, PrefilteredBitmaps> bitmaps_to_evaluate;
   for (size_t i = 0; i < database.partitions.size(); ++i) {
      const DatabasePartition& database_partition = database.partitions.at(i);
      OperatorResult& filter = bitmap_filter[i];
      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == database_partition.sequence_count) {
         for (const auto& [aa_name, aa_store] : database_partition.aa_sequences) {
            bitmaps_to_evaluate[aa_name].full_bitmaps.emplace_back(filter, aa_store);
         }
      } else {
         if (filter.isMutable()) {
            filter->runOptimize();
         }
         for (const auto& [aa_name, aa_store] : database_partition.aa_sequences) {
            bitmaps_to_evaluate[aa_name].bitmaps.emplace_back(filter, aa_store);
         }
      }
   }
   return bitmaps_to_evaluate;
}

void AAMutations::addMutationsCountsForPosition(
   uint32_t position,
   const PrefilteredBitmaps& bitmaps_to_evaluate,
   SymbolMap<AminoAcid, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   for (const auto& [filter, aa_store_partition] : bitmaps_to_evaluate.bitmaps) {
      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (aa_store_partition.positions[position].symbol_whose_bitmap_is_flipped != symbol) {
            count_of_mutations_per_position[symbol][position] +=
               filter->and_cardinality(aa_store_partition.positions[position].bitmaps.at(symbol));
         } else {
            count_of_mutations_per_position[symbol][position] +=
               filter->andnot_cardinality(aa_store_partition.positions[position].bitmaps.at(symbol)
               );
         }
      }
   }
   // For these partitions, we have full bitmaps. Do not need to bother with AND
   // cardinality
   for (const auto& [filter, aa_store_partition] : bitmaps_to_evaluate.full_bitmaps) {
      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (aa_store_partition.positions[position].symbol_whose_bitmap_is_flipped != symbol) {
            count_of_mutations_per_position[symbol][position] +=
               aa_store_partition.positions[position].bitmaps.at(symbol).cardinality();
         } else {
            count_of_mutations_per_position[symbol][position] +=
               aa_store_partition.sequence_count -
               aa_store_partition.positions[position].bitmaps.at(symbol).cardinality();
         }
      }
   }
}

SymbolMap<AminoAcid, std::vector<uint32_t>> AAMutations::calculateMutationsPerPosition(
   const SequenceStore<AminoAcid>& aa_store,
   const PrefilteredBitmaps& bitmap_filter
) {
   const size_t sequence_length = aa_store.reference_sequence.size();

   SymbolMap<AminoAcid, std::vector<uint32_t>> count_of_mutations_per_position;
   for (const auto symbol : VALID_MUTATION_SYMBOLS) {
      count_of_mutations_per_position[symbol].resize(sequence_length);
   }
   static constexpr int POSITIONS_PER_PROCESS = 300;
   tbb::parallel_for(
      tbb::blocked_range<uint32_t>(0, sequence_length, /*grain_size=*/POSITIONS_PER_PROCESS),
      [&](const auto& local) {
         for (uint32_t pos = local.begin(); pos != local.end(); ++pos) {
            addMutationsCountsForPosition(pos, bitmap_filter, count_of_mutations_per_position);
         }
      }
   );
   return count_of_mutations_per_position;
}

void AAMutations::validateOrderByFields(const Database& /*database*/) const {
   const std::vector<std::string> result_field_names{
      {MUTATION_FIELD_NAME, PROPORTION_FIELD_NAME, COUNT_FIELD_NAME}};

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

void AAMutations::addMutationsToOutput(
   const std::string& sequence_name,
   const SequenceStore<AminoAcid>& aa_store,
   const PrefilteredBitmaps& bitmap_filter,
   std::vector<QueryResultEntry>& output
) const {
   const size_t sequence_length = aa_store.reference_sequence.size();

   const SymbolMap<AminoAcid, std::vector<uint32_t>> count_of_mutations_per_position =
      calculateMutationsPerPosition(aa_store, bitmap_filter);

   for (size_t pos = 0; pos < sequence_length; ++pos) {
      uint32_t total = 0;
      for (const AminoAcid::Symbol symbol : VALID_MUTATION_SYMBOLS) {
         total += count_of_mutations_per_position.at(symbol)[pos];
      }
      if (total == 0) {
         continue;
      }
      const auto threshold_count =
         static_cast<uint32_t>(std::ceil(static_cast<double>(total) * min_proportion) - 1);

      const AminoAcid::Symbol symbol_in_reference_genome = aa_store.reference_sequence.at(pos);

      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (symbol_in_reference_genome != symbol) {
            const uint32_t count = count_of_mutations_per_position.at(symbol)[pos];
            if (count > threshold_count) {
               const double proportion = static_cast<double>(count) / static_cast<double>(total);
               const std::
                  map<std::string, std::optional<std::variant<std::string, int32_t, double>>>
                     fields{
                        {MUTATION_FIELD_NAME,
                         AminoAcid::symbolToChar(symbol_in_reference_genome) +
                            std::to_string(pos + 1) + AminoAcid::symbolToChar(symbol)},
                        {SEQUENCE_FIELD_NAME, sequence_name},
                        {PROPORTION_FIELD_NAME, proportion},
                        {COUNT_FIELD_NAME, static_cast<int32_t>(count)}};
               output.push_back({fields});
            }
         }
      }
   }
}

QueryResult AAMutations::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   std::vector<std::string> aa_sequence_names_to_evaluate;
   for (const auto& aa_sequence_name : aa_sequence_names) {
      CHECK_SILO_QUERY(
         database.aa_sequences.contains(aa_sequence_name),
         "Database does not contain the amino acid sequence with name: '" + aa_sequence_name + "'"
      )
      aa_sequence_names_to_evaluate.emplace_back(aa_sequence_name);
   }
   if (aa_sequence_names.empty()) {
      for (const auto& [aa_sequence_name, _] : database.aa_sequences) {
         aa_sequence_names_to_evaluate.emplace_back(aa_sequence_name);
      }
   }

   std::unordered_map<std::string, AAMutations::PrefilteredBitmaps> bitmaps_to_evaluate =
      preFilterBitmaps(database, bitmap_filter);

   std::vector<QueryResultEntry> mutation_proportions;
   for (const auto& aa_sequence_name : aa_sequence_names_to_evaluate) {
      const SequenceStore<AminoAcid>& aa_store = database.aa_sequences.at(aa_sequence_name);

      addMutationsToOutput(
         aa_sequence_name, aa_store, bitmaps_to_evaluate.at(aa_sequence_name), mutation_proportions
      );
   }
   return {mutation_proportions};
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<AAMutations>& action) {
   CHECK_SILO_QUERY(
      !json.contains("sequenceName") ||
         (json["sequenceName"].is_string() || json["sequenceName"].is_array()),
      "AminoAcidMutations action can have the field sequenceName of type string or an array of "
      "strings, but no other type"
   )
   std::vector<std::string> sequence_names;
   if (json.contains("sequenceName") && json["sequenceName"].is_array()) {
      for (const auto& child : json["sequenceName"]) {
         CHECK_SILO_QUERY(
            child.is_string(),
            "The field sequenceName of AminoAcidMutations action must have type string or an "
            "array, if present. Found:" +
               child.dump()
         )
         sequence_names.emplace_back(child.get<std::string>());
      }
   } else if (json.contains("sequenceName") && json["sequenceName"].is_string()) {
      sequence_names.emplace_back(json["sequenceName"].get<std::string>());
   }

   double min_proportion = AAMutations::DEFAULT_MIN_PROPORTION;
   if (json.contains("minProportion")) {
      min_proportion = json["minProportion"].get<double>();
      if (min_proportion <= 0 || min_proportion > 1) {
         throw QueryParseException(
            "Invalid proportion: minProportion must be in interval (0.0, 1.0]"
         );
      }
   }
   action = std::make_unique<AAMutations>(std::move(sequence_names), min_proportion);
}

}  // namespace silo::query_engine::actions
