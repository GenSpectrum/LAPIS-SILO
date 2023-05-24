#include "silo/query_engine/query_engine.h"

#include <silo/common/block_timer.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <cmath>

#include "silo/common/log.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/database.h"
#include "silo/storage/database_partition.h"

uint64_t silo::executeCount(
   const silo::Database& /*database*/,
   std::vector<silo::query_engine::OperatorResult>& partition_filters
) {
   std::atomic<uint32_t> count = 0;
   tbb::parallel_for_each(partition_filters.begin(), partition_filters.end(), [&](auto& filter) {
      count += filter.getConst()->cardinality();
      filter.free();
   });
   return count;
}
// TODO(someone): reduce cognitive complexity
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
std::vector<silo::MutationProportion> silo::executeMutations(
   const silo::Database& database,
   std::vector<silo::query_engine::OperatorResult>& partition_filters,
   double proportion_threshold
) {
   using roaring::Roaring;

   std::vector<uint32_t> count_of_nucleotide_symbols_a_at_position(silo::GENOME_LENGTH);
   std::vector<uint32_t> count_of_nucleotide_symbols_c_at_position(silo::GENOME_LENGTH);
   std::vector<uint32_t> count_of_nucleotide_symbols_g_at_position(silo::GENOME_LENGTH);
   std::vector<uint32_t> count_of_nucleotide_symbols_t_at_position(silo::GENOME_LENGTH);
   std::vector<uint32_t> count_of_gaps_at_position(silo::GENOME_LENGTH);

   std::vector<unsigned> partition_filters_to_evaluate;
   std::vector<unsigned> full_partition_filters_to_evaluate;
   for (unsigned i = 0; i < database.partitions.size(); ++i) {
      const silo::DatabasePartition& dbp = database.partitions[i];
      silo::query_engine::OperatorResult filter = partition_filters[i];
      const Roaring& bitmap = *filter.getConst();
      // TODO(taepper) check naive run_compression
      const unsigned card = bitmap.cardinality();
      if (card == 0) {
         continue;
      }
      if (card == dbp.sequenceCount) {
         full_partition_filters_to_evaluate.push_back(i);
      } else {
         if (filter.isMutable()) {
            filter.getMutable()->runOptimize();
         }
         partition_filters_to_evaluate.push_back(i);
      }
   }

   int64_t microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      static constexpr int POSITIONS_PER_PROCESS = 300;
      tbb::blocked_range<uint32_t> const range(
         0, silo::GENOME_LENGTH, /*grain_size=*/POSITIONS_PER_PROCESS
      );
      // TODO(someone): reduce cognitive complexity
      // NOLINTNEXTLINE(readability-function-cognitive-complexity)
      tbb::parallel_for(range.begin(), range.end(), [&](uint32_t pos) {
         for (unsigned const partition_index : partition_filters_to_evaluate) {
            const silo::DatabasePartition& database_partition =
               database.partitions[partition_index];
            silo::query_engine::OperatorResult const filter = partition_filters[partition_index];
            const Roaring& bitmap = *filter.getConst();

            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::A) {
               count_of_nucleotide_symbols_a_at_position[pos] += bitmap.and_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::A)]
               );
            } else {
               count_of_nucleotide_symbols_a_at_position[pos] += bitmap.andnot_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::A)]
               );
            }
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::C) {
               count_of_nucleotide_symbols_c_at_position[pos] += bitmap.and_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::C)]
               );
            } else {
               count_of_nucleotide_symbols_c_at_position[pos] += bitmap.andnot_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::C)]
               );
            }
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::G) {
               count_of_nucleotide_symbols_g_at_position[pos] += bitmap.and_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::G)]
               );
            } else {
               count_of_nucleotide_symbols_g_at_position[pos] += bitmap.andnot_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::G)]
               );
            }
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::T) {
               count_of_nucleotide_symbols_t_at_position[pos] += bitmap.and_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::T)]
               );
            } else {
               count_of_nucleotide_symbols_t_at_position[pos] += bitmap.andnot_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::T)]
               );
            }
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::GAP) {
               count_of_gaps_at_position[pos] += bitmap.and_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::GAP)]
               );
            } else {
               count_of_gaps_at_position[pos] += bitmap.andnot_cardinality(
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::GAP)]
               );
            }
         }
         // For these partitions, we have full bitmaps. Do not need to bother with AND cardinality
         for (unsigned const partition_index : full_partition_filters_to_evaluate) {
            const silo::DatabasePartition& database_partition =
               database.partitions[partition_index];
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::A) {
               count_of_nucleotide_symbols_a_at_position[pos] +=
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::A)]
                     .cardinality();
            } else {
               count_of_nucleotide_symbols_a_at_position[pos] +=
                  database_partition.sequenceCount -
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::A)]
                     .cardinality();
            }
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::C) {
               count_of_nucleotide_symbols_c_at_position[pos] +=
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::C)]
                     .cardinality();
            } else {
               count_of_nucleotide_symbols_c_at_position[pos] +=
                  database_partition.sequenceCount -
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::C)]
                     .cardinality();
            }
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::G) {
               count_of_nucleotide_symbols_g_at_position[pos] +=
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::G)]
                     .cardinality();
            } else {
               count_of_nucleotide_symbols_g_at_position[pos] +=
                  database_partition.sequenceCount -
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::G)]
                     .cardinality();
            }
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::T) {
               count_of_nucleotide_symbols_t_at_position[pos] +=
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::T)]
                     .cardinality();
            } else {
               count_of_nucleotide_symbols_t_at_position[pos] +=
                  database_partition.sequenceCount -
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::T)]
                     .cardinality();
            }
            if (database_partition.seq_store.positions[pos].symbol_whose_bitmap_is_flipped != silo::NUCLEOTIDE_SYMBOL::GAP) {
               count_of_gaps_at_position[pos] +=
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::GAP)]
                     .cardinality();
            } else {
               count_of_gaps_at_position[pos] +=
                  database_partition.sequenceCount -
                  database_partition.seq_store.positions[pos]
                     .bitmaps[static_cast<unsigned>(silo::NUCLEOTIDE_SYMBOL::GAP)]
                     .cardinality();
            }
         }
      });
   }
   LOG_PERFORMANCE("Position calculation: {} microseconds", std::to_string(microseconds));

   for (unsigned i = 0; i < database.partitions.size(); ++i) {
      partition_filters[i].free();
   }

   std::vector<silo::MutationProportion> mutation_proportions;
   microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      for (unsigned pos = 0; pos < silo::GENOME_LENGTH; ++pos) {
         uint32_t const total = count_of_nucleotide_symbols_a_at_position[pos] +
                                count_of_nucleotide_symbols_c_at_position[pos] +
                                count_of_nucleotide_symbols_g_at_position[pos] +
                                count_of_nucleotide_symbols_t_at_position[pos] +
                                count_of_gaps_at_position[pos];
         if (total == 0) {
            continue;
         }
         auto const threshold_count =
            static_cast<uint32_t>(std::ceil(static_cast<double>(total) * proportion_threshold) - 1);

         char const pos_ref = database.global_reference[0].at(pos);
         if (pos_ref != 'A') {
            const uint32_t tmp = count_of_nucleotide_symbols_a_at_position[pos];
            if (tmp > threshold_count) {
               double const proportion = static_cast<double>(tmp) / static_cast<double>(total);
               mutation_proportions.push_back({pos_ref, pos, 'A', proportion, tmp});
            }
         }
         if (pos_ref != 'C') {
            const uint32_t tmp = count_of_nucleotide_symbols_c_at_position[pos];
            if (tmp > threshold_count) {
               double const proportion = static_cast<double>(tmp) / static_cast<double>(total);
               mutation_proportions.push_back({pos_ref, pos, 'C', proportion, tmp});
            }
         }
         if (pos_ref != 'G') {
            const uint32_t tmp = count_of_nucleotide_symbols_g_at_position[pos];
            if (tmp > threshold_count) {
               double const proportion = static_cast<double>(tmp) / static_cast<double>(total);
               mutation_proportions.push_back({pos_ref, pos, 'G', proportion, tmp});
            }
         }
         if (pos_ref != 'T') {
            const uint32_t tmp = count_of_nucleotide_symbols_t_at_position[pos];
            if (tmp > threshold_count) {
               double const proportion = static_cast<double>(tmp) / static_cast<double>(total);
               mutation_proportions.push_back({pos_ref, pos, 'T', proportion, tmp});
            }
         }
         // This should always be the case. For future-proof-ness (gaps in reference), keep this
         // check in.
         if (pos_ref != '-') {
            const uint32_t tmp = count_of_gaps_at_position[pos];
            if (tmp > threshold_count) {
               double const proportion = static_cast<double>(tmp) / static_cast<double>(total);
               mutation_proportions.push_back({pos_ref, pos, '-', proportion, tmp});
            }
         }
      }
   }
   LOG_PERFORMANCE("Proportion calculation: {} microseconds", std::to_string(microseconds));

   return mutation_proportions;
}
