#include "silo/query_engine/actions/nuc_mutations.h"

#include <cmath>
#include <map>

#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <nlohmann/json.hpp>

#include "silo/common/block_timer.h"
#include "silo/common/log.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/query_engine/operator_result.h"
#include "silo/query_engine/query_parse_exception.h"
#include "silo/query_engine/query_result.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/reference_genome.h"

namespace silo::query_engine::actions {

NucMutations::NucMutations(double min_proportion)
    : min_proportion(min_proportion) {}

std::array<std::vector<uint32_t>, 5> NucMutations::calculateMutationsPerPosition(
   const Database& database,
   std::vector<OperatorResult>& bitmap_filter
) const {
   std::vector<size_t> bitmap_filters_to_evaluate;
   std::vector<size_t> full_bitmap_filters_to_evaluate;
   for (size_t i = 0; i < database.partitions.size(); ++i) {
      const silo::DatabasePartition& dbp = database.partitions[i];
      silo::query_engine::OperatorResult& filter = bitmap_filter[i];
      const size_t cardinality = filter->cardinality();
      if (cardinality == 0) {
         continue;
      }
      if (cardinality == dbp.sequenceCount) {
         full_bitmap_filters_to_evaluate.push_back(i);
      } else {
         if (filter.isMutable()) {
            filter->runOptimize();
         }
         bitmap_filters_to_evaluate.push_back(i);
      }
   }

   std::array<std::vector<uint32_t>, 5> count_of_mutations_per_position{
      std::vector<uint32_t>(GENOME_LENGTH),
      std::vector<uint32_t>(GENOME_LENGTH),
      std::vector<uint32_t>(GENOME_LENGTH),
      std::vector<uint32_t>(GENOME_LENGTH),
      std::vector<uint32_t>(GENOME_LENGTH)};
   static constexpr int POSITIONS_PER_PROCESS = 300;
   const tbb::blocked_range<uint32_t> range(
      0, silo::GENOME_LENGTH, /*grain_size=*/POSITIONS_PER_PROCESS
   );
   tbb::parallel_for(range.begin(), range.end(), [&](uint32_t pos) {
      for (const unsigned partition_index : bitmap_filters_to_evaluate) {
         const silo::DatabasePartition& database_partition = database.partitions[partition_index];
         const silo::query_engine::OperatorResult& filter = bitmap_filter[partition_index];

         for (silo::NUCLEOTIDE_SYMBOL symbol : VALID_MUTATION_SYMBOLS) {
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != symbol) {
               count_of_mutations_per_position[static_cast<unsigned>(symbol)][pos] +=
                  filter->and_cardinality(database_partition.seq_store.positions[pos]
                                             .bitmaps[static_cast<unsigned>(symbol)]);
            } else {
               count_of_mutations_per_position[static_cast<unsigned>(symbol)][pos] +=
                  filter->andnot_cardinality(database_partition.seq_store.positions[pos]
                                                .bitmaps[static_cast<unsigned>(symbol)]);
            }
         }
      }
      // For these partitions, we have full bitmaps. Do not need to bother with AND cardinality
      for (unsigned const partition_index : full_bitmap_filters_to_evaluate) {
         const silo::DatabasePartition& database_partition = database.partitions[partition_index];

         for (silo::NUCLEOTIDE_SYMBOL symbol : VALID_MUTATION_SYMBOLS) {
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != symbol) {
               count_of_mutations_per_position[static_cast<unsigned>(symbol)][pos] +=
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(symbol)]
                     .cardinality();
            } else {
               count_of_mutations_per_position[static_cast<unsigned>(symbol)][pos] +=
                  database_partition.sequenceCount - database_partition.seq_store.positions[pos]
                                                        .bitmaps[static_cast<unsigned>(symbol)]
                                                        .cardinality();
            }
         }
      }
   });
   return count_of_mutations_per_position;
}

QueryResult NucMutations::execute(
   const Database& database,
   std::vector<OperatorResult> bitmap_filter
) const {
   using roaring::Roaring;

   std::array<std::vector<uint32_t>, 5> count_of_mutations_per_position =
      calculateMutationsPerPosition(database, bitmap_filter);

   std::vector<silo::query_engine::QueryResultEntry> mutation_proportions;
   {
      for (unsigned pos = 0; pos < silo::GENOME_LENGTH; ++pos) {
         const uint32_t total =
            count_of_mutations_per_position[0][pos] + count_of_mutations_per_position[1][pos] +
            count_of_mutations_per_position[2][pos] + count_of_mutations_per_position[3][pos] +
            count_of_mutations_per_position[4][pos];
         if (total == 0) {
            continue;
         }
         const auto threshold_count =
            static_cast<uint32_t>(std::ceil(static_cast<double>(total) * min_proportion) - 1);

         const auto pos_ref =
            toNucleotideSymbol(database.reference_genome->genome_segments[0].at(pos));

         for (silo::NUCLEOTIDE_SYMBOL symbol : VALID_MUTATION_SYMBOLS) {
            if (pos_ref != symbol) {
               const uint32_t count =
                  count_of_mutations_per_position[static_cast<size_t>(symbol)][pos];
               if (count > threshold_count) {
                  const double proportion = static_cast<double>(count) / static_cast<double>(total);
                  std::map<std::string, std::variant<std::string, int32_t, double>> fields{
                     {"position",
                      SYMBOL_REPRESENTATION[static_cast<size_t>(pos_ref)] +
                         std::to_string(pos + 1) +
                         SYMBOL_REPRESENTATION[static_cast<size_t>(symbol)]},
                     {"proportion", proportion},
                     {"count", static_cast<int32_t>(count)}};
                  mutation_proportions.push_back({fields});
               }
            }
         }
      }
   }

   return {mutation_proportions};
}

void from_json(const nlohmann::json& json, std::unique_ptr<NucMutations>& action) {
   double min_proportion = NucMutations::DEFAULT_MIN_PROPORTION;
   if (json.contains("minProportion")) {
      min_proportion = json["minProportion"].get<double>();
      if (min_proportion <= 0 || min_proportion > 1) {
         throw QueryParseException(
            "Invalid proportion: minProportion must be in interval (0.0, 1.0]"
         );
      }
   }
   action = std::make_unique<NucMutations>(min_proportion);
}

}  // namespace silo::query_engine::actions
