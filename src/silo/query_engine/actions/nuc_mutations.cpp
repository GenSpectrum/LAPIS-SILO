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
   const silo::SequenceStore<Nucleotide>& seq_store,
   std::vector<OperatorResult>& bitmap_filter
) {
   PrefilteredBitmaps bitmaps_to_evaluate;
   for (size_t i = 0; i < seq_store.partitions.size(); ++i) {
      const silo::SequenceStorePartition<Nucleotide>& seq_store_partition =
         seq_store.partitions.at(i);
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
   SymbolMap<Nucleotide, std::vector<uint32_t>>& count_of_mutations_per_position
) {
   for (const auto& [filter, seq_store_partition] : bitmaps_to_evaluate.bitmaps) {
      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (seq_store_partition.positions[position].symbol_whose_bitmap_is_flipped != symbol) {
            count_of_mutations_per_position[symbol][position] +=
               filter->and_cardinality(seq_store_partition.positions[position].bitmaps.at(symbol));
         } else {
            count_of_mutations_per_position[symbol][position] +=
               filter->andnot_cardinality(seq_store_partition.positions[position].bitmaps.at(symbol)
               );
         }
      }
   }
   // For these partitions, we have full bitmaps. Do not need to bother with AND
   // cardinality
   for (const auto& [filter, seq_store_partition] : bitmaps_to_evaluate.full_bitmaps) {
      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (seq_store_partition.positions[position].symbol_whose_bitmap_is_flipped != symbol) {
            count_of_mutations_per_position[symbol][position] +=
               seq_store_partition.positions[position].bitmaps.at(symbol).cardinality();
         } else {
            count_of_mutations_per_position[symbol][position] +=
               seq_store_partition.sequence_count -
               seq_store_partition.positions[position].bitmaps.at(symbol).cardinality();
         }
      }
   }
}

SymbolMap<Nucleotide, std::vector<uint32_t>> NucMutations::calculateMutationsPerPosition(
   const SequenceStore<Nucleotide>& seq_store,
   std::vector<OperatorResult>& bitmap_filter
) {
   const size_t genome_length = seq_store.reference_sequence.size();

   PrefilteredBitmaps bitmaps_to_evaluate = preFilterBitmaps(seq_store, bitmap_filter);

   SymbolMap<Nucleotide, std::vector<uint32_t>> count_of_mutations_per_position;
   for (const Nucleotide::Symbol symbol : VALID_MUTATION_SYMBOLS) {
      count_of_mutations_per_position[symbol].resize(genome_length);
   }
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

void NucMutations::validateOrderByFields(const Database& /*database*/) const {
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

QueryResult NucMutations::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   const std::string nuc_sequence_name_or_default =
      nuc_sequence_name.value_or(database.database_config.default_nucleotide_sequence);
   CHECK_SILO_QUERY(
      database.nuc_sequences.contains(nuc_sequence_name_or_default),
      "Database does not contain the nucleotide sequence with name: '" +
         nuc_sequence_name_or_default + "'"
   )

   const SequenceStore<Nucleotide>& seq_store =
      database.nuc_sequences.at(nuc_sequence_name_or_default);

   const size_t genome_length = seq_store.reference_sequence.size();

   const SymbolMap<Nucleotide, std::vector<uint32_t>> count_of_mutations_per_position =
      calculateMutationsPerPosition(seq_store, bitmap_filter);

   std::vector<QueryResultEntry> mutation_proportions;
   for (size_t pos = 0; pos < genome_length; ++pos) {
      uint32_t total = 0;
      for (const Nucleotide::Symbol symbol : VALID_MUTATION_SYMBOLS) {
         total += count_of_mutations_per_position.at(symbol)[pos];
      }
      if (total == 0) {
         continue;
      }
      const auto threshold_count =
         static_cast<uint32_t>(std::ceil(static_cast<double>(total) * min_proportion) - 1);

      const Nucleotide::Symbol symbol_in_reference_genome = seq_store.reference_sequence.at(pos);

      for (const auto symbol : VALID_MUTATION_SYMBOLS) {
         if (symbol_in_reference_genome != symbol) {
            const uint32_t count = count_of_mutations_per_position.at(symbol)[pos];
            if (count > threshold_count) {
               const double proportion = static_cast<double>(count) / static_cast<double>(total);
               const std::
                  map<std::string, std::optional<std::variant<std::string, int32_t, double>>>
                     fields{
                        {MUTATION_FIELD_NAME,
                         Nucleotide::symbolToChar(symbol_in_reference_genome) +
                            std::to_string(pos + 1) + Nucleotide::symbolToChar(symbol)},
                        {PROPORTION_FIELD_NAME, proportion},
                        {COUNT_FIELD_NAME, static_cast<int32_t>(count)}};
               mutation_proportions.push_back({fields});
            }
         }
      }
   }

   return {mutation_proportions};
}

// NOLINTNEXTLINE(readability-identifier-naming)
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
