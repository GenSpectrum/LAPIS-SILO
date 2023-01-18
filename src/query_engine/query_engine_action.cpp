//
// Created by Alexander Taepper on 08.01.23.
//

#include "silo/query_engine/query_engine.h"
#include <cmath>
#include <silo/common/PerfEvent.hpp>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>

uint64_t silo::execute_count(const silo::Database& /*db*/, std::vector<silo::filter_t>& partition_filters) {
   std::atomic<uint32_t> count = 0;
   tbb::parallel_for_each(partition_filters.begin(), partition_filters.end(), [&](auto& filter) {
      count += filter.getAsConst()->cardinality();
      filter.free();
   });
   return count;
}

std::vector<silo::mutation_proportion> silo::execute_mutations(const silo::Database& db, std::vector<silo::filter_t>& partition_filters, double proportion_threshold) {
   using roaring::Roaring;

   std::vector<uint32_t> A_per_pos(silo::genomeLength);
   std::vector<uint32_t> C_per_pos(silo::genomeLength);
   std::vector<uint32_t> G_per_pos(silo::genomeLength);
   std::vector<uint32_t> T_per_pos(silo::genomeLength);
   std::vector<uint32_t> gap_per_pos(silo::genomeLength);

   std::vector<unsigned> partition_filters_to_evaluate;
   std::vector<unsigned> full_partition_filters_to_evaluate;
   for (unsigned i = 0; i < db.partitions.size(); ++i) {
      const silo::DatabasePartition& dbp = db.partitions[i];
      silo::filter_t filter = partition_filters[i];
      const Roaring& bm = *filter.getAsConst();
      if (bm.isEmpty()) {
         continue;
      } else if (bm.cardinality() == dbp.sequenceCount) {
         full_partition_filters_to_evaluate.push_back(i);
      } else {
         // TODO check if run-compression is beneficial here
         partition_filters_to_evaluate.push_back(i);
      }
   }

   int64_t microseconds = 0;
   {
      BlockTimer timer(microseconds);

      tbb::blocked_range<uint32_t> range(0, silo::genomeLength, /*grain_size=*/300);
      tbb::parallel_for(range.begin(), range.end(), [&](uint32_t pos) {
         for (unsigned i : partition_filters_to_evaluate) {
            const silo::DatabasePartition& dbp = db.partitions[i];
            silo::filter_t filter = partition_filters[i];
            const Roaring& bm = *filter.getAsConst();

            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::A) { /// everything fine
               A_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::A]);
            } else { /// Bitmap was flipped
               A_per_pos[pos] += bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::A]);
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::C) { /// everything fine
               C_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::C]);
            } else { /// Bitmap was flipped
               C_per_pos[pos] += bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::C]);
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::G) { /// everything fine
               G_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::G]);
            } else { /// Bitmap was flipped
               G_per_pos[pos] += bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::G]);
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::T) { /// everything fine
               T_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::T]);
            } else { /// Bitmap was flipped
               T_per_pos[pos] += bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::T]);
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::gap) { /// everything fine
               gap_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::gap]);
            } else { /// Bitmap was flipped
               gap_per_pos[pos] += bm.andnot_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::gap]);
            }
         }
         /// For these partitions, we have full bitmaps. Do not need to bother with AND cardinality
         for (unsigned i : full_partition_filters_to_evaluate) {
            const silo::DatabasePartition& dbp = db.partitions[i];
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::A) { /// everything fine
               A_per_pos[pos] += dbp.seq_store.positions[pos].bitmaps[silo::Symbol::A].cardinality();
            } else { /// Bitmap was flipped
               A_per_pos[pos] += dbp.sequenceCount - dbp.seq_store.positions[pos].bitmaps[silo::Symbol::A].cardinality();
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::C) { /// everything fine
               C_per_pos[pos] += dbp.seq_store.positions[pos].bitmaps[silo::Symbol::C].cardinality();
            } else { /// Bitmap was flipped
               C_per_pos[pos] += dbp.sequenceCount - dbp.seq_store.positions[pos].bitmaps[silo::Symbol::C].cardinality();
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::G) { /// everything fine
               G_per_pos[pos] += dbp.seq_store.positions[pos].bitmaps[silo::Symbol::G].cardinality();
            } else { /// Bitmap was flipped
               G_per_pos[pos] += dbp.sequenceCount - dbp.seq_store.positions[pos].bitmaps[silo::Symbol::G].cardinality();
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::T) { /// everything fine
               T_per_pos[pos] += dbp.seq_store.positions[pos].bitmaps[silo::Symbol::T].cardinality();
            } else { /// Bitmap was flipped
               T_per_pos[pos] += dbp.sequenceCount - dbp.seq_store.positions[pos].bitmaps[silo::Symbol::T].cardinality();
            }
            if (dbp.seq_store.positions[pos].flipped_bitmap != silo::Symbol::gap) { /// everything fine
               gap_per_pos[pos] += dbp.seq_store.positions[pos].bitmaps[silo::Symbol::gap].cardinality();
            } else { /// Bitmap was flipped
               gap_per_pos[pos] += dbp.sequenceCount - dbp.seq_store.positions[pos].bitmaps[silo::Symbol::gap].cardinality();
            }
         }
      });
   }
   // std::cerr << "Per pos calculation: " << std::to_string(microseconds) << std::endl;

   for (unsigned i = 0; i < db.partitions.size(); ++i) {
      partition_filters[i].free();
   }

   std::vector<silo::mutation_proportion> ret;
   microseconds = 0;
   {
      BlockTimer timer(microseconds);
      for (unsigned pos = 0; pos < silo::genomeLength; ++pos) {
         uint32_t total = A_per_pos[pos] + C_per_pos[pos] + G_per_pos[pos] + T_per_pos[pos] + gap_per_pos[pos];
         if (total == 0) {
            continue;
         }
         uint32_t threshold_count = std::ceil((double) total * (double) proportion_threshold) - 1;

         char pos_ref = db.global_reference[0].at(pos);
         if (pos_ref != 'A') {
            const uint32_t tmp = A_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref, pos, 'A', proportion, tmp});
            }
         }
         if (pos_ref != 'C') {
            const uint32_t tmp = C_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref, pos, 'C', proportion, tmp});
            }
         }
         if (pos_ref != 'G') {
            const uint32_t tmp = G_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref, pos, 'G', proportion, tmp});
            }
         }
         if (pos_ref != 'T') {
            const uint32_t tmp = T_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref, pos, 'T', proportion, tmp});
            }
         }
         /// This should always be the case. For future-proof-ness (gaps in reference), keep this check in.
         if (pos_ref != '-') {
            const uint32_t tmp = gap_per_pos[pos];
            if (tmp > threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref, pos, '-', proportion, tmp});
            }
         }
      }
   }
   // std::cerr << "Proportion / ret calculation: " << std::to_string(microseconds) << std::endl;

   return ret;
}