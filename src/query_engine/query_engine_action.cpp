#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <cmath>
#include <silo/common/PerfEvent.hpp>
#include "silo/query_engine/query_engine.h"

uint64_t silo::executeCount(
   const silo::Database& database /*db*/,
   std::vector<silo::BooleanExpressionResult>& partition_filters
) {
   std::atomic<uint32_t> count = 0;
   tbb::parallel_for_each(partition_filters.begin(), partition_filters.end(), [&](auto& filter) {
      count += filter.getAsConst()->cardinality();
      filter.free();
   });
   return count;
}

std::vector<silo::MutationProportion> silo::executeMutations(
   const silo::Database& database,
   std::vector<silo::BooleanExpressionResult>& partition_filters,
   double proportion_threshold,
   std::ostream& performance_file
) {
   using roaring::Roaring;

   std::vector<uint32_t> A_per_pos(silo::GENOME_LENGTH);
   std::vector<uint32_t> C_per_pos(silo::GENOME_LENGTH);
   std::vector<uint32_t> G_per_pos(silo::GENOME_LENGTH);
   std::vector<uint32_t> T_per_pos(silo::GENOME_LENGTH);
   std::vector<uint32_t> gap_per_pos(silo::GENOME_LENGTH);

   std::vector<unsigned> partition_filters_to_evaluate;
   std::vector<unsigned> full_partition_filters_to_evaluate;
   for (unsigned i = 0; i < database.partitions.size(); ++i) {
      const silo::DatabasePartition& dbp = database.partitions[i];
      silo::BooleanExpressionResult filter = partition_filters[i];
      const Roaring& bm = *filter.getAsConst();
      // TODO check naive run_compression
      const unsigned card = bm.cardinality();
      if (card == 0) {
         continue;
      } else if (card == dbp.sequenceCount) {
         full_partition_filters_to_evaluate.push_back(i);
      } else {
         if (filter.mutable_res) {
            filter.mutable_res->runOptimize();
         }
         partition_filters_to_evaluate.push_back(i);
      }
   }

   int64_t microseconds = 0;
   {
      BlockTimer timer(microseconds);

      tbb::blocked_range<uint32_t> range(0, silo::GENOME_LENGTH, /*grain_size=*/300);
      tbb::parallel_for(range.begin(), range.end(), [&](uint32_t pos) {
         for (unsigned i : partition_filters_to_evaluate) {
            const silo::DatabasePartition& dbp = database.partitions[i];
            silo::BooleanExpressionResult filter = partition_filters[i];
            const Roaring& bm = *filter.getAsConst();

            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::A) {  /// everything
                                                                                          /// fine
               A_per_pos[pos] +=
                  bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::A]);
            } else {  /// Bitmap was flipped
               A_per_pos[pos] +=
                  bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::A]
                  );
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::C) {  /// everything
                                                                                          /// fine
               C_per_pos[pos] +=
                  bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::C]);
            } else {  /// Bitmap was flipped
               C_per_pos[pos] +=
                  bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::C]
                  );
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::G) {  /// everything
                                                                                          /// fine
               G_per_pos[pos] +=
                  bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::G]);
            } else {  /// Bitmap was flipped
               G_per_pos[pos] +=
                  bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::G]
                  );
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::T) {  /// everything
                                                                                          /// fine
               T_per_pos[pos] +=
                  bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::T]);
            } else {  /// Bitmap was flipped
               T_per_pos[pos] +=
                  bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::T]
                  );
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::GAP) {  /// everything
                                                                                            /// fine
               gap_per_pos[pos] +=
                  bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::GAP]
                  );
            } else {  /// Bitmap was flipped
               gap_per_pos[pos] += bm.andnot_cardinality(
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::GAP]
               );
            }
         }
         /// For these partitions, we have full bitmaps. Do not need to bother with AND cardinality
         for (unsigned i : full_partition_filters_to_evaluate) {
            const silo::DatabasePartition& dbp = database.partitions[i];
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::A) {  /// everything
                                                                                          /// fine
               A_per_pos[pos] +=
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::A].cardinality();
            } else {  /// Bitmap was flipped
               A_per_pos[pos] +=
                  dbp.sequenceCount -
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::A].cardinality();
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::C) {  /// everything
                                                                                          /// fine
               C_per_pos[pos] +=
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::C].cardinality();
            } else {  /// Bitmap was flipped
               C_per_pos[pos] +=
                  dbp.sequenceCount -
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::C].cardinality();
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::G) {  /// everything
                                                                                          /// fine
               G_per_pos[pos] +=
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::G].cardinality();
            } else {  /// Bitmap was flipped
               G_per_pos[pos] +=
                  dbp.sequenceCount -
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::G].cardinality();
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::T) {  /// everything
                                                                                          /// fine
               T_per_pos[pos] +=
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::T].cardinality();
            } else {  /// Bitmap was flipped
               T_per_pos[pos] +=
                  dbp.sequenceCount -
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::T].cardinality();
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::GENOME_SYMBOL::GAP) {  /// everything
                                                                                            /// fine
               gap_per_pos[pos] +=
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::GAP].cardinality();
            } else {  /// Bitmap was flipped
               gap_per_pos[pos] +=
                  dbp.sequenceCount -
                  dbp.seq_store.positions[pos].bitmaps[silo::GENOME_SYMBOL::GAP].cardinality();
            }
         }
      });
   }
   performance_file << "pos_calculation\t" << std::to_string(microseconds) << std::endl;

   for (unsigned i = 0; i < database.partitions.size(); ++i) {
      partition_filters[i].free();
   }

   std::vector<silo::MutationProportion> mutation_proportions;
   microseconds = 0;
   {
      BlockTimer timer(microseconds);
      for (unsigned pos = 0; pos < silo::GENOME_LENGTH; ++pos) {
         uint32_t total =
            A_per_pos[pos] + C_per_pos[pos] + G_per_pos[pos] + T_per_pos[pos] + gap_per_pos[pos];
         if (total == 0) {
            continue;
         }
         uint32_t threshold_count = std::ceil((double)total * (double)proportion_threshold) - 1;

         char pos_ref = database.global_reference[0].at(pos);
         if (pos_ref != 'A') {
            const uint32_t tmp = A_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double)tmp / (double)total;
               mutation_proportions.push_back({pos_ref, pos, 'A', proportion, tmp});
            }
         }
         if (pos_ref != 'C') {
            const uint32_t tmp = C_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double)tmp / (double)total;
               mutation_proportions.push_back({pos_ref, pos, 'C', proportion, tmp});
            }
         }
         if (pos_ref != 'G') {
            const uint32_t tmp = G_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double)tmp / (double)total;
               mutation_proportions.push_back({pos_ref, pos, 'G', proportion, tmp});
            }
         }
         if (pos_ref != 'T') {
            const uint32_t tmp = T_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double)tmp / (double)total;
               mutation_proportions.push_back({pos_ref, pos, 'T', proportion, tmp});
            }
         }
         /// This should always be the case. For future-proof-ness (gaps in reference), keep this
         /// check in.
         if (pos_ref != '-') {
            const uint32_t tmp = gap_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double)tmp / (double)total;
               mutation_proportions.push_back({pos_ref, pos, '-', proportion, tmp});
            }
         }
      }
   }
   performance_file << "Proportion_calculation\t" << std::to_string(microseconds) << std::endl;

   return mutation_proportions;
}
