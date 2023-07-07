#include "silo/query_engine/actions/nuc_mutations.h"

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
#include <nlohmann/json.hpp>
#include <roaring/roaring.hh>

#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/database.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/sequence_store.h"

using silo::query_engine::OperatorResult;

namespace silo::query_engine::actions {

NucMutations::NucMutations(std::optional<std::string> nuc_sequence_name, double min_proportion)
    : nuc_sequence_name(std::move(nuc_sequence_name)),
      min_proportion(min_proportion) {}

NucMutations::PrefilteredBitmaps NucMutations::preFilterBitmaps(
   const silo::SequenceStore& seq_store,
   std::vector<OperatorResult>& bitmap_filter
) {
   PrefilteredBitmaps bitmaps_to_evaluate;
   for (size_t i = 0; i < seq_store.partitions.size(); ++i) {
      const silo::SequenceStorePartition& seq_store_partition = seq_store.partitions.at(i);
      OperatorResult& filter = bitmap_filter[i];
      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == seq_store_partition.sequence_count) {
         bitmaps_to_evaluate.full_bitmaps.emplace_back(std::move(filter), seq_store_partition);
      } else {
         if (filter.isMutable()) {
            filter->runOptimize();
         }
         bitmaps_to_evaluate.bitmaps.emplace_back(std::move(filter), seq_store_partition);
      }
   }
   return bitmaps_to_evaluate;
}

void NucMutations::addMutationsCountsForPosition(
   uint32_t position,
   PrefilteredBitmaps& bitmaps_to_evaluate,
   std::array<std::vector<uint32_t>, MUTATION_SYMBOL_COUNT>& count_of_mutations_per_position
) {
   for (auto& [filter, seq_store_partition] : bitmaps_to_evaluate.bitmaps) {
      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (seq_store_partition.positions[position].symbol_whose_bitmap_is_flipped != symbol) {
            count_of_mutations_per_position[static_cast<uint32_t>(symbol)][position] +=
               filter->and_cardinality(
                  seq_store_partition.positions[position].bitmaps[static_cast<uint32_t>(symbol)]
               );
         } else {
            count_of_mutations_per_position[static_cast<uint32_t>(symbol)][position] +=
               filter->andnot_cardinality(
                  seq_store_partition.positions[position].bitmaps[static_cast<uint32_t>(symbol)]
               );
         }
      }
   }
   // For these partitions, we have full bitmaps. Do not need to bother with AND
   // cardinality
   for (auto& [filter, seq_store_partition] : bitmaps_to_evaluate.full_bitmaps) {
      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (seq_store_partition.positions[position].symbol_whose_bitmap_is_flipped != symbol) {
            count_of_mutations_per_position[static_cast<uint32_t>(symbol)][position] +=
               seq_store_partition.positions[position]
                  .bitmaps[static_cast<uint32_t>(symbol)]
                  .cardinality();
         } else {
            count_of_mutations_per_position[static_cast<uint32_t>(symbol)][position] +=
               seq_store_partition.sequence_count - seq_store_partition.positions[position]
                                                       .bitmaps[static_cast<uint32_t>(symbol)]
                                                       .cardinality();
         }
      }
   }
}

std::array<std::vector<uint32_t>, NucMutations::MUTATION_SYMBOL_COUNT> NucMutations::
   calculateMutationsPerPosition(
      const SequenceStore& seq_store,
      std::vector<OperatorResult>& bitmap_filter
   ) {
   const size_t genome_length = seq_store.reference_genome.length();

   PrefilteredBitmaps bitmaps_to_evaluate = preFilterBitmaps(seq_store, bitmap_filter);

   std::array<std::vector<uint32_t>, MUTATION_SYMBOL_COUNT> count_of_mutations_per_position{
      std::vector<uint32_t>(genome_length),
      std::vector<uint32_t>(genome_length),
      std::vector<uint32_t>(genome_length),
      std::vector<uint32_t>(genome_length),
      std::vector<uint32_t>(genome_length)};
   static constexpr int POSITIONS_PER_PROCESS = 300;
   tbb::parallel_for(
      tbb::blocked_range<uint32_t>(0, genome_length, /*grain_size=*/POSITIONS_PER_PROCESS).begin(),
      tbb::blocked_range<uint32_t>(0, genome_length, /*grain_size=*/POSITIONS_PER_PROCESS).end(),
      [&](uint32_t pos) {
         // for ( = local.begin(); pos != local.end(); ++pos) {
         addMutationsCountsForPosition(pos, bitmaps_to_evaluate, count_of_mutations_per_position);
         //}
      }
   );
   return count_of_mutations_per_position;
}

QueryResult NucMutations::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   using roaring::Roaring;
   const std::string nuc_sequence_name_or_default =
      nuc_sequence_name.value_or(database.database_config.default_nucleotide_sequence);
   CHECK_SILO_QUERY(
      database.nuc_sequences.contains(nuc_sequence_name_or_default),
      "Database does not contain the nucleotide sequence with name: '" +
         nuc_sequence_name_or_default + "'"
   )

   const SequenceStore& seq_store = database.nuc_sequences.at(nuc_sequence_name_or_default);

   const size_t genome_length = seq_store.reference_genome.length();

   std::array<std::vector<uint32_t>, MUTATION_SYMBOL_COUNT> count_of_mutations_per_position =
      calculateMutationsPerPosition(seq_store, bitmap_filter);

   std::vector<QueryResultEntry> mutation_proportions;
   for (size_t pos = 0; pos < genome_length; ++pos) {
      const uint32_t total =
         count_of_mutations_per_position[0][pos] + count_of_mutations_per_position[1][pos] +
         count_of_mutations_per_position[2][pos] + count_of_mutations_per_position[3][pos] +
         count_of_mutations_per_position[4][pos];
      if (total == 0) {
         continue;
      }
      const auto threshold_count =
         static_cast<uint32_t>(std::ceil(static_cast<double>(total) * min_proportion) - 1);

      const auto symbol_in_reference_genome =
         toNucleotideSymbol(seq_store.reference_genome.at(pos)).value();

      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (symbol_in_reference_genome != symbol) {
            const uint32_t count =
               count_of_mutations_per_position[static_cast<size_t>(symbol)][pos];
            if (count > threshold_count) {
               const double proportion = static_cast<double>(count) / static_cast<double>(total);
               const std::map<
                  std::string,
                  std::optional<std::variant<std::string, int32_t, double>>>
                  fields{
                     {"position",
                      NUC_SYMBOL_REPRESENTATION[static_cast<size_t>(symbol_in_reference_genome)] +
                         std::to_string(pos + 1) +
                         NUC_SYMBOL_REPRESENTATION[static_cast<size_t>(symbol)]},
                     {"proportion", proportion},
                     {"count", static_cast<int32_t>(count)}};
               mutation_proportions.push_back({fields});
            }
         }
      }
   }

   return {mutation_proportions};
}

void from_json(const nlohmann::json& json, std::unique_ptr<NucMutations>& action) {
   double min_proportion = NucMutations::DEFAULT_MIN_PROPORTION;
   std::optional<std::string> nuc_sequence_name;
   if (json.contains("sequenceName")) {
      nuc_sequence_name = json["sequenceName"].get<std::string>();
   }
   if (json.contains("minProportion")) {
      min_proportion = json["minProportion"].get<double>();
      if (min_proportion <= 0 || min_proportion > 1) {
         throw QueryParseException(
            "Invalid proportion: minProportion must be in interval (0.0, 1.0]"
         );
      }
   }
   action = std::make_unique<NucMutations>(nuc_sequence_name, min_proportion);
}

}  // namespace silo::query_engine::actions
