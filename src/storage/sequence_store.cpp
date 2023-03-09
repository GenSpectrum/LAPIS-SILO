#include <silo/database.h>
#include <silo/storage/sequence_store.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <silo/common/PerfEvent.hpp>
#include <syncstream>

using namespace silo;

roaring::Roaring* SequenceStore::bma(size_t pos, GENOME_SYMBOL r) const {
   switch (r) {
      case A: {
         const roaring::Roaring* tmp[8] = {bm(pos, A), bm(pos, R), bm(pos, W), bm(pos, M),
                                           bm(pos, D), bm(pos, H), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case C: {
         const roaring::Roaring* tmp[8] = {bm(pos, C), bm(pos, Y), bm(pos, S), bm(pos, M),
                                           bm(pos, B), bm(pos, H), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case G: {
         const roaring::Roaring* tmp[8] = {bm(pos, G), bm(pos, R), bm(pos, S), bm(pos, K),
                                           bm(pos, D), bm(pos, B), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case T: {
         const roaring::Roaring* tmp[8] = {bm(pos, T), bm(pos, Y), bm(pos, W), bm(pos, K),
                                           bm(pos, D), bm(pos, H), bm(pos, B)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      default: {
         return new roaring::Roaring(*bm(pos, r));
      }
   }
}

roaring::Roaring* SequenceStore::bma_neg(size_t pos, GENOME_SYMBOL r) const {
   auto tmp1 = bm(pos, r);
   roaring::api::roaring_bitmap_flip(&tmp1->roaring, 0, sequence_count);
   switch (r) {
      case A: {
         const roaring::Roaring* tmp[8] = {tmp1,       bm(pos, R), bm(pos, W), bm(pos, M),
                                           bm(pos, D), bm(pos, H), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case C: {
         const roaring::Roaring* tmp[8] = {tmp1,       bm(pos, Y), bm(pos, S), bm(pos, M),
                                           bm(pos, B), bm(pos, H), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case G: {
         const roaring::Roaring* tmp[8] = {tmp1,       bm(pos, R), bm(pos, S), bm(pos, K),
                                           bm(pos, D), bm(pos, B), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case T: {
         const roaring::Roaring* tmp[8] = {tmp1,       bm(pos, Y), bm(pos, W), bm(pos, K),
                                           bm(pos, D), bm(pos, H), bm(pos, B)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      default: {
         return new roaring::Roaring(*bm(pos, r));
      }
   }
}

int SequenceStore::db_info(std::ostream& io) const {
   std::osyncstream(io) << "partition sequence count: " << formatNumber(this->sequence_count)
                        << std::endl;
   std::osyncstream(io) << "partition index size: " << formatNumber(computeSize()) << std::endl;

   size_t size = 0;
   for (auto& r : N_bitmaps) {
      size += r.getSizeInBytes(false);
   }
   std::osyncstream(io) << "partition N_bitmap per sequence, total size: " << formatNumber(size)
                        << std::endl;
   return 0;
}

/// Appends the sequences in genome to the current bitmaps in SequenceStore and increases
/// sequenceCount
void SequenceStore::interpret(const std::vector<std::string>& genomes) {
   const uint32_t cur_sequence_count = sequence_count;
   sequence_count += genomes.size();
   N_bitmaps.resize(cur_sequence_count + genomes.size());
   {
      tbb::blocked_range<unsigned> range(0, GENOME_LENGTH, GENOME_LENGTH / 64);
      tbb::parallel_for(range, [&](const decltype(range)& local) {
         /// For every symbol, calculate all sequence IDs that have that symbol at that position
         std::vector<std::vector<unsigned>> ids_per_symbol(SYMBOL_COUNT);
         for (unsigned col = local.begin(); col != local.end(); ++col) {
            for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
               char c = genomes[index2][col];
               GENOME_SYMBOL s = toNucleotideSymbol(c);
               if (s != GENOME_SYMBOL::N)
                  ids_per_symbol[s].push_back(cur_sequence_count + index2);
            }
            for (unsigned symbol = 0; symbol != SYMBOL_COUNT; ++symbol)
               if (!ids_per_symbol[symbol].empty()) {
                  this->positions[col].bitmaps[symbol].addMany(
                     ids_per_symbol[symbol].size(), ids_per_symbol[symbol].data()
                  );
                  ids_per_symbol[symbol].clear();
               }
         }
      });
   }
   {
      tbb::blocked_range<unsigned> range(0, genomes.size());
      tbb::parallel_for(range, [&](const decltype(range)& local) {
         /// For every symbol, calculate all sequence IDs that have that symbol at that position
         std::vector<unsigned> N_positions(SYMBOL_COUNT);
         for (unsigned genome = local.begin(); genome != local.end(); ++genome) {
            for (unsigned pos = 0, limit2 = GENOME_LENGTH; pos != limit2; ++pos) {
               char c = genomes[genome][pos];
               GENOME_SYMBOL s = toNucleotideSymbol(c);
               if (s == GENOME_SYMBOL::N)
                  N_positions.push_back(pos);
            }
            if (!N_positions.empty()) {
               this->N_bitmaps[cur_sequence_count + genome].addMany(
                  N_positions.size(), N_positions.data()
               );
               this->N_bitmaps[cur_sequence_count + genome].runOptimize();
               N_positions.clear();
            }
         }
      });
   }
}

void SequenceStore::indexAllN() {
   std::vector<std::vector<std::vector<uint32_t>>> ids_per_position_per_upper(
      (sequence_count >> 16) + 1
   );
   tbb::blocked_range<uint32_t> range(0, (sequence_count >> 16) + 1);
   tbb::parallel_for(range.begin(), range.end(), [&](uint32_t local) {
      auto& ids_per_position = ids_per_position_per_upper[local];
      ids_per_position.resize(GENOME_LENGTH);

      uint32_t genome_upper = local << 16;
      uint32_t limit =
         genome_upper == (sequence_count & 0xFFFF0000) ? sequence_count - genome_upper : 1u << 16;
      for (uint32_t genome_lower = 0; genome_lower < limit; ++genome_lower) {
         const uint32_t genome = genome_upper | genome_lower;
         for (uint32_t pos : N_bitmaps[genome]) {
            ids_per_position[pos].push_back(genome);
         }
      }
   });

   for (uint32_t pos = 0; pos < GENOME_LENGTH; ++pos) {
      for (uint32_t upper = 0; upper < (sequence_count >> 16) + 1; ++upper) {
         auto& v = ids_per_position_per_upper[upper][pos];
         positions[pos].bitmaps[GENOME_SYMBOL::N].addMany(v.size(), v.data());
      }
      positions[pos].N_indexed = true;
   }
}

void SequenceStore::indexAllN_naive() {
   tbb::enumerable_thread_specific<std::vector<std::vector<uint32_t>>> ids_per_position;
   tbb::blocked_range<uint32_t> range(0, (sequence_count >> 16) + 1);
   tbb::parallel_for(range.begin(), range.end(), [&](uint32_t local) {
      ids_per_position.local().resize(GENOME_LENGTH);

      uint32_t genome_upper = local << 16;
      uint32_t limit =
         genome_upper == (sequence_count & 0xFFFF0000) ? sequence_count - genome_upper : 1u << 16;
      for (uint32_t genome_lower = 0; genome_lower < limit; ++genome_lower) {
         const uint32_t genome = genome_upper | genome_lower;
         for (uint32_t pos : N_bitmaps[genome]) {
            ids_per_position.local()[pos].push_back(genome);
         }
      }
   });

   for (auto& v1 : ids_per_position) {
      for (uint32_t pos = 0; pos < GENOME_LENGTH; ++pos) {
         auto& v = v1[pos];
         positions[pos].bitmaps[GENOME_SYMBOL::N].addMany(v.size(), v.data());
      }
   }

   for (uint32_t pos = 0; pos < GENOME_LENGTH; ++pos) {
      positions[pos].N_indexed = true;
   }
}

[[maybe_unused]] unsigned silo::runOptimize(SequenceStore& db) {
   std::atomic<unsigned> count_true = 0;
   tbb::blocked_range<Position*> r(std::begin(db.positions), std::end(db.positions));
   tbb::parallel_for(r, [&](const decltype(r) local) {
      for (Position& p : local) {
         for (auto& bm : p.bitmaps) {
            if (bm.runOptimize())
               ++count_true;
         }
      }
   });
   return count_true;
}

[[maybe_unused]] unsigned silo::shrinkToFit(SequenceStore& db) {
   std::atomic<size_t> saved = 0;
   tbb::blocked_range<Position*> r(std::begin(db.positions), std::end(db.positions));
   tbb::parallel_for(r, [&](const decltype(r) local) {
      size_t local_saved = 0;
      for (Position& p : local) {
         for (auto& bm : p.bitmaps) {
            local_saved += bm.shrinkToFit();
         }
      }
      saved += local_saved;
   });
   return saved;
}
