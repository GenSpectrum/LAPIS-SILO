#include "silo/query_engine/actions/aa_mutations.h"

#include <cmath>
#include <deque>
#include <map>
#include <optional>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "silo/common/aa_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/aa_store.h"

using silo::query_engine::OperatorResult;

namespace silo::query_engine::actions {

AAMutations::AAMutations(std::string aa_sequence_name, double min_proportion)
    : aa_sequence_name(std::move(aa_sequence_name)),
      min_proportion(min_proportion) {}

AAMutations::PrefilteredBitmaps AAMutations::preFilterBitmaps(
   const silo::AAStore& aa_store,
   std::vector<OperatorResult>& bitmap_filter
) {
   PrefilteredBitmaps bitmaps_to_evaluate;
   for (size_t i = 0; i < aa_store.partitions.size(); ++i) {
      const silo::AAStorePartition& aa_store_partition = aa_store.partitions.at(i);
      OperatorResult& filter = bitmap_filter[i];
      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == aa_store_partition.sequence_count) {
         bitmaps_to_evaluate.full_bitmaps.emplace_back(std::move(filter), aa_store_partition);
      } else {
         if (filter.isMutable()) {
            filter->runOptimize();
         }
         bitmaps_to_evaluate.bitmaps.emplace_back(std::move(filter), aa_store_partition);
      }
   }
   return bitmaps_to_evaluate;
}

void AAMutations::addMutationsCountsForPosition(
   uint32_t position,
   PrefilteredBitmaps& bitmaps_to_evaluate,
   NucleotideSymbolMap<std::vector<uint32_t>>& count_of_mutations_per_position
) {
   for (auto& [filter, aa_store_partition] : bitmaps_to_evaluate.bitmaps) {
      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (aa_store_partition.positions[position].symbol_whose_bitmap_is_flipped != symbol) {
            count_of_mutations_per_position.at(symbol)[position] +=
               filter->and_cardinality(
                  aa_store_partition.positions[position].bitmaps[static_cast<uint32_t>(symbol)]
               );
         } else {
            count_of_mutations_per_position.at(symbol)[position] +=
               filter->andnot_cardinality(
                  aa_store_partition.positions[position].bitmaps[static_cast<uint32_t>(symbol)]
               );
         }
      }
   }
   // For these partitions, we have full bitmaps. Do not need to bother with AND
   // cardinality
   for (auto& [filter, aa_store_partition] : bitmaps_to_evaluate.full_bitmaps) {
      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (aa_store_partition.positions[position].symbol_whose_bitmap_is_flipped != symbol) {
            count_of_mutations_per_position.at(symbol)[position] +=
               aa_store_partition.positions[position]
                  .bitmaps[static_cast<uint32_t>(symbol)]
                  .cardinality();
         } else {
            count_of_mutations_per_position.at(symbol)[position] +=
               aa_store_partition.sequence_count - aa_store_partition.positions[position]
                                                      .bitmaps.at(symbol)
                                                      .cardinality();
         }
      }
   }
}

NucleotideSymbolMap<std::vector<uint32_t>> AAMutations::
   calculateMutationsPerPosition(
      const AAStore& aa_store,
      std::vector<OperatorResult>& bitmap_filter
   ) {
   const size_t sequence_length = aa_store.reference_sequence.size();

   PrefilteredBitmaps bitmaps_to_evaluate = preFilterBitmaps(aa_store, bitmap_filter);

   NucleotideSymbolMap<std::vector<uint32_t>> count_of_mutations_per_position;
   for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         count_of_mutations_per_position[symbol].resize(sequence_length);
   }
   static constexpr int POSITIONS_PER_PROCESS = 300;
   tbb::parallel_for(
      tbb::blocked_range<uint32_t>(0, sequence_length, /*grain_size=*/POSITIONS_PER_PROCESS),
      [&](const auto& local) {
         for (uint32_t pos = local.begin(); pos != local.end(); ++pos) {
            addMutationsCountsForPosition(
               pos, bitmaps_to_evaluate, count_of_mutations_per_position
            );
         }
      }
   );
   return count_of_mutations_per_position;
}

QueryResult AAMutations::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   using roaring::Roaring;
   CHECK_SILO_QUERY(
      database.aa_sequences.contains(aa_sequence_name),
      "Database does not contain the amino acid sequence with name: '" + aa_sequence_name + "'"
   )

   const AAStore& aa_store = database.aa_sequences.at(aa_sequence_name);

   const size_t sequence_length = aa_store.reference_sequence.size();

   const AASymbolMap<std::vector<uint32_t>> count_of_mutations_per_position =
      calculateMutationsPerPosition(aa_store, bitmap_filter);

   std::vector<QueryResultEntry> mutation_proportions;
   for (size_t pos = 0; pos < sequence_length; ++pos) {
      uint32_t total = 0;
      for (const AA_SYMBOL symbol : VALID_MUTATION_SYMBOLS) {
         total += count_of_mutations_per_position.at(symbol)[pos];
      }
      if (total == 0) {
         continue;
      }
      const auto threshold_count =
         static_cast<uint32_t>(std::ceil(static_cast<double>(total) * min_proportion) - 1);

      const AA_SYMBOL symbol_in_reference_genome = aa_store.reference_sequence.at(pos);

      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (symbol_in_reference_genome != symbol) {
            const uint32_t count = count_of_mutations_per_position.at(symbol)[pos];
            if (count > threshold_count) {
               const double proportion = static_cast<double>(count) / static_cast<double>(total);
               const std::
                  map<std::string, std::optional<std::variant<std::string, int32_t, double>>>
                     fields{
                        {"position",
                         aaSymbolToChar(symbol_in_reference_genome) + std::to_string(pos + 1) +
                            aaSymbolToChar(symbol)},
                        {"proportion", proportion},
                        {"count", static_cast<int32_t>(count)}};
               mutation_proportions.push_back({fields});
            }
         }
      }
   }

   return {mutation_proportions};
}

// NOLINTNEXTLINE(readability-identifier-naming)
void from_json(const nlohmann::json& json, std::unique_ptr<AAMutations>& action) {
   CHECK_SILO_QUERY(
      json.contains("sequenceName") && json["sequenceName"].is_string(),
      "AminoAcideMutations action must have the field sequenceName:string"
   )
   const std::string aa_sequence_name = json["sequenceName"].get<std::string>();
   double min_proportion = AAMutations::DEFAULT_MIN_PROPORTION;
   if (json.contains("minProportion")) {
      min_proportion = json["minProportion"].get<double>();
      if (min_proportion <= 0 || min_proportion > 1) {
         throw QueryParseException(
            "Invalid proportion: minProportion must be in interval (0.0, 1.0]"
         );
      }
   }
   action = std::make_unique<AAMutations>(aa_sequence_name, min_proportion);
}

}  // namespace silo::query_engine::actions
