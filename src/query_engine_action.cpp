//
// Created by Alexander Taepper on 08.01.23.
//

#include "silo/query_engine.h"
#include <silo/common/PerfEvent.hpp>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>

uint64_t execute_count(const silo::Database& /*db*/, std::vector<silo::filter_t>& partition_filters) {
   std::atomic<uint32_t> count = 0;
   tbb::parallel_for_each(partition_filters.begin(), partition_filters.end(), [&](auto& filter) {
      count += filter.getAsConst()->cardinality();
      filter.free();
   });
   return count;
}

std::vector<silo::mut_struct> execute_mutations(const silo::Database& db, std::vector<silo::filter_t>& partition_filters, double proportion_threshold) {
   using roaring::Roaring;

   std::vector<uint32_t> N_per_pos(silo::genomeLength);
   std::vector<uint32_t> C_per_pos(silo::genomeLength);
   std::vector<uint32_t> T_per_pos(silo::genomeLength);
   std::vector<uint32_t> A_per_pos(silo::genomeLength);
   std::vector<uint32_t> G_per_pos(silo::genomeLength);
   std::vector<uint32_t> gap_per_pos(silo::genomeLength);

   int64_t microseconds = 0;
   {
      BlockTimer timer(microseconds);

      tbb::blocked_range<uint32_t> range(0, silo::genomeLength, /*grainsize=*/300);
      tbb::parallel_for(range.begin(), range.end(), [&](uint32_t pos) {
         for (unsigned i = 0; i < db.partitions.size(); ++i) {
            const silo::DatabasePartition& dbp = db.partitions[i];
            silo::filter_t filter = partition_filters[i];
            const Roaring& bm = *filter.getAsConst();

            N_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::N]);

            char pos_ref = db.global_reference[0].at(pos);
            if (pos_ref != 'C') {
               if (dbp.seq_store.positions[pos].reference != silo::Symbol::C) { /// everything fine
                  C_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::C]);
               } else { /// Bitmap was flipped
                  C_per_pos[pos] += dbp.sequenceCount - bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::C]);
               }
            }
            if (pos_ref != 'T') {
               if (dbp.seq_store.positions[pos].reference != silo::Symbol::T) { /// everything fine
                  T_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::T]);
               } else { /// Bitmap was flipped
                  T_per_pos[pos] += dbp.sequenceCount - bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::T]);
               }
            }
            if (pos_ref != 'A') {
               if (dbp.seq_store.positions[pos].reference != silo::Symbol::A) { /// everything fine
                  A_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::A]);
               } else { /// Bitmap was flipped
                  A_per_pos[pos] += dbp.sequenceCount - bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::A]);
               }
            }
            if (pos_ref != 'G') {
               if (dbp.seq_store.positions[pos].reference != silo::Symbol::G) { /// everything fine
                  G_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::G]);
               } else { /// Bitmap was flipped
                  G_per_pos[pos] += dbp.sequenceCount - bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::G]);
               }
            }
            if (pos_ref == '-') {
               if (dbp.seq_store.positions[pos].reference != silo::Symbol::gap) { /// everything fine
                  gap_per_pos[pos] += bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::gap]);
               } else { /// Bitmap was flipped
                  gap_per_pos[pos] += dbp.sequenceCount - bm.and_cardinality(dbp.seq_store.positions[pos].bitmaps[silo::Symbol::gap]);
               }
            }
         }
      });
   }
   std::cerr << "Per pos calculation: " << std::to_string(microseconds) << std::endl;

   uint32_t sequence_count = 0;
   for (unsigned i = 0; i < db.partitions.size(); ++i) {
      sequence_count += partition_filters[i].getAsConst()->cardinality();
      partition_filters[i].free();
   }

   std::vector<silo::mut_struct> ret;
   microseconds = 0;
   {
      BlockTimer timer(microseconds);
      for (unsigned pos = 0; pos < silo::genomeLength; ++pos) {
         char pos_ref = db.global_reference[0].at(pos);
         std::vector<std::pair<char, uint32_t>> candidates;
         uint32_t total = sequence_count - N_per_pos[pos];
         uint32_t threshold_count = total * proportion_threshold;
         if (pos_ref != 'C') {
            const uint32_t tmp = C_per_pos[pos];
            if (tmp >= threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref + std::to_string(pos + 1) + 'C', proportion, tmp});
            }
         }
         if (pos_ref != 'T') {
            const uint32_t tmp = T_per_pos[pos];
            if (tmp >= threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref + std::to_string(pos + 1) + 'T', proportion, tmp});
            }
         }
         if (pos_ref != 'A') {
            const uint32_t tmp = A_per_pos[pos];
            if (tmp >= threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref + std::to_string(pos + 1) + 'A', proportion, tmp});
            }
         }
         if (pos_ref != 'G') {
            const uint32_t tmp = G_per_pos[pos];
            if (tmp >= threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref + std::to_string(pos + 1) + 'G', proportion, tmp});
            }
         }
         /// This should always be the case. For future-proof-ness (gaps in reference), keep this check in.
         if (pos_ref != '-') {
            const uint32_t tmp = gap_per_pos[pos];
            if (tmp >= threshold_count) {
               double proportion = (double) tmp / (double) total;
               ret.push_back({pos_ref + std::to_string(pos + 1) + '-', proportion, tmp});
            }
         }
      }
   }
   std::cerr << "Proportion / ret calculation: " << std::to_string(microseconds) << std::endl;

   return ret;
}