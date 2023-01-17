//
// Created by Alexander Taepper on 01.09.22.
//

#include <silo/common/PerfEvent.hpp>
#include <syncstream>
#include <silo/storage/sequence_store.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>

using namespace silo;

roaring::Roaring* SequenceStore::bma(size_t pos, Symbol r) const {
   switch (r) {
      case A: {
         const roaring::Roaring* tmp[8] = {bm(pos, A),
                                           bm(pos, R), bm(pos, W), bm(pos, M),
                                           bm(pos, D), bm(pos, H), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case C: {
         const roaring::Roaring* tmp[8] = {bm(pos, C),
                                           bm(pos, Y), bm(pos, S), bm(pos, M),
                                           bm(pos, B), bm(pos, H), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case G: {
         const roaring::Roaring* tmp[8] = {bm(pos, G),
                                           bm(pos, R), bm(pos, S), bm(pos, K),
                                           bm(pos, D), bm(pos, B), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case T: {
         const roaring::Roaring* tmp[8] = {bm(pos, T),
                                           bm(pos, Y), bm(pos, W), bm(pos, K),
                                           bm(pos, D), bm(pos, H), bm(pos, B)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      default: {
         return new roaring::Roaring(*bm(pos, r));
      }
   }
}

roaring::Roaring* SequenceStore::bma_neg(size_t pos, Symbol r) const {
   auto tmp1 = bm(pos, r);
   roaring::api::roaring_bitmap_flip(&tmp1->roaring, 0, sequence_count);
   switch (r) {
      case A: {
         const roaring::Roaring* tmp[8] = {tmp1,
                                           bm(pos, R), bm(pos, W), bm(pos, M),
                                           bm(pos, D), bm(pos, H), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case C: {
         const roaring::Roaring* tmp[8] = {tmp1,
                                           bm(pos, Y), bm(pos, S), bm(pos, M),
                                           bm(pos, B), bm(pos, H), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case G: {
         const roaring::Roaring* tmp[8] = {tmp1,
                                           bm(pos, R), bm(pos, S), bm(pos, K),
                                           bm(pos, D), bm(pos, B), bm(pos, V)};
         roaring::Roaring* ret = new roaring::Roaring(roaring::Roaring::fastunion(8, tmp));
         return ret;
      }
      case T: {
         const roaring::Roaring* tmp[8] = {tmp1,
                                           bm(pos, Y), bm(pos, W), bm(pos, K),
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
   std::osyncstream(io) << "partition sequence count: " << number_fmt(this->sequence_count) << std::endl;
   std::osyncstream(io) << "partition index size: " << number_fmt(computeSize()) << std::endl;

   size_t size = 0;
   for (auto& r : N_bitmaps) {
      size += r.getSizeInBytes(false);
   }
   std::osyncstream(io) << "partition N_bitmap per sequence, total size: " << number_fmt(size) << std::endl;
   return 0;
}

/*  Legacy interpret, now interpret without N, N may manually be indexed
/// Appends the sequences in genome to the current bitmaps in SequenceStore and increases sequenceCount
void SequenceStore::interpret(const std::vector<std::string>& genomes) {
   const uint32_t cur_sequence_count = sequence_count;
   sequence_count += genomes.size();
   N_bitmaps.resize(cur_sequence_count + genomes.size());
   {
      tbb::blocked_range<unsigned> range(0, genomeLength, genomeLength / 64);
      tbb::parallel_for(range, [&](const decltype(range)& local) {
         /// For every symbol, calculate all sequence IDs that have that symbol at that position
         std::vector<std::vector<unsigned>> ids_per_symbol(symbolCount);
         for (unsigned col = local.begin(); col != local.end(); ++col) {
            for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
               char c = genomes[index2][col];
               Symbol s = to_symbol(c);
               if (s != Symbol::N)
                  ids_per_symbol[s].push_back(cur_sequence_count + index2);
            }
            for (unsigned symbol = 0; symbol != symbolCount; ++symbol)
               if (!ids_per_symbol[symbol].empty()) {
                  this->positions[col].bitmaps[symbol].addMany(ids_per_symbol[symbol].size(), ids_per_symbol[symbol].data());
                  ids_per_symbol[symbol].clear();
               }
         }
      });
   }
} */

/// Appends the sequences in genome to the current bitmaps in SequenceStore and increases sequenceCount
void SequenceStore::interpret(const std::vector<std::string>& genomes) {
   const uint32_t cur_sequence_count = sequence_count;
   sequence_count += genomes.size();
   N_bitmaps.resize(cur_sequence_count + genomes.size());
   {
      tbb::blocked_range<unsigned> range(0, genomeLength, genomeLength / 64);
      tbb::parallel_for(range, [&](const decltype(range)& local) {
         /// For every symbol, calculate all sequence IDs that have that symbol at that position
         std::vector<std::vector<unsigned>> ids_per_symbol(symbolCount);
         for (unsigned col = local.begin(); col != local.end(); ++col) {
            for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
               char c = genomes[index2][col];
               Symbol s = to_symbol(c);
               if (s != Symbol::N)
                  ids_per_symbol[s].push_back(cur_sequence_count + index2);
            }
            for (unsigned symbol = 0; symbol != symbolCount; ++symbol)
               if (!ids_per_symbol[symbol].empty()) {
                  this->positions[col].bitmaps[symbol].addMany(ids_per_symbol[symbol].size(), ids_per_symbol[symbol].data());
                  ids_per_symbol[symbol].clear();
               }
         }
      });
   }
   {
      tbb::blocked_range<unsigned> range(0, genomes.size());
      tbb::parallel_for(range, [&](const decltype(range)& local) {
         /// For every symbol, calculate all sequence IDs that have that symbol at that position
         std::vector<unsigned> N_positions(symbolCount);
         for (unsigned genome = local.begin(); genome != local.end(); ++genome) {
            for (unsigned pos = 0, limit2 = genomeLength; pos != limit2; ++pos) {
               char c = genomes[genome][pos];
               Symbol s = to_symbol(c);
               if (s == Symbol::N)
                  N_positions.push_back(pos);
            }
            if (!N_positions.empty()) {
               this->N_bitmaps[cur_sequence_count + genome].addMany(N_positions.size(), N_positions.data());
               this->N_bitmaps[cur_sequence_count + genome].runOptimize();
               N_positions.clear();
            }
         }
      });
   }
}

void SequenceStore::indexAllN() {
   std::vector<std::vector<std::vector<uint32_t>>> ids_per_position_per_upper((sequence_count >> 16) + 1);
   tbb::blocked_range<uint32_t> range(0, (sequence_count >> 16) + 1);
   tbb::parallel_for(range.begin(), range.end(), [&](uint32_t local) {
      auto& ids_per_position = ids_per_position_per_upper[local];
      ids_per_position.resize(genomeLength);

      uint32_t genome_upper = local << 16;
      uint32_t limit = genome_upper == (sequence_count & 0xFFFF0000) ? sequence_count - genome_upper : 1u << 16;
      for (uint32_t genome_lower = 0; genome_lower < limit; ++genome_lower) {
         const uint32_t genome = genome_upper | genome_lower;
         for (uint32_t pos : N_bitmaps[genome]) {
            ids_per_position[pos].push_back(genome);
         }
      }
   });

   for (uint32_t pos = 0; pos < genomeLength; ++pos) {
      for (uint32_t upper = 0; upper < (sequence_count >> 16) + 1; ++upper) {
         auto& v = ids_per_position_per_upper[upper][pos];
         positions[pos].bitmaps[Symbol::N].addMany(v.size(), v.data());
      }
      positions[pos].N_indexed = true;
   }
}

void SequenceStore::indexAllN_naive() {
   tbb::enumerable_thread_specific<std::vector<std::vector<uint32_t>>> ids_per_position;
   tbb::blocked_range<uint32_t> range(0, (sequence_count >> 16) + 1);
   tbb::parallel_for(range.begin(), range.end(), [&](uint32_t local) {
      ids_per_position.local().resize(genomeLength);

      uint32_t genome_upper = local << 16;
      uint32_t limit = genome_upper == (sequence_count & 0xFFFF0000) ? sequence_count - genome_upper : 1u << 16;
      for (uint32_t genome_lower = 0; genome_lower < limit; ++genome_lower) {
         const uint32_t genome = genome_upper | genome_lower;
         for (uint32_t pos : N_bitmaps[genome]) {
            ids_per_position.local()[pos].push_back(genome);
         }
      }
   });

   for (auto& v1 : ids_per_position) {
      for (uint32_t pos = 0; pos < genomeLength; ++pos) {
         auto& v = v1[pos];
         positions[pos].bitmaps[Symbol::N].addMany(v.size(), v.data());
      }
   }

   for (uint32_t pos = 0; pos < genomeLength; ++pos) {
      positions[pos].N_indexed = true;
   }
}

[[maybe_unused]] unsigned silo::runOptimize(SequenceStore& db) {
   std::atomic<unsigned> count_true = 0;
   tbb::blocked_range<Position*> r(std::begin(db.positions), std::end(db.positions));
   tbb::parallel_for(r, [&](const decltype(r) local) {
      for (Position& p : local) {
         for (auto& bm : p.bitmaps) {
            if (bm.runOptimize()) ++count_true;
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

CompressedSequenceStore::CompressedSequenceStore(const SequenceStore& seq_store) {
   using roaring::Roaring;

   this->sequence_count = seq_store.sequence_count;

   start_gaps.resize(seq_store.sequence_count);
   end_gaps.resize(seq_store.sequence_count);

   for (unsigned i = 0; i < genomeLength; i++) {
      positions[i].flipped_bitmap = seq_store.positions[i].flipped_bitmap;
      for (uint16_t s = 0; s < symbolCount; s++) {
         positions[i].bitmaps[s] = seq_store.positions[i].bitmaps[s];
      }
   }
   {
      Roaring current_gap_extension_candidates;
      current_gap_extension_candidates.addRange(0, seq_store.sequence_count);
      for (unsigned i = 0; i < genomeLength / 4; i++) {
         current_gap_extension_candidates &= seq_store.positions[i].bitmaps[Symbol::gap];
         roaring::api::roaring_bitmap_andnot_inplace(&this->positions[i].bitmaps[Symbol::gap].roaring, &current_gap_extension_candidates.roaring);
         uint32_t flipped_bitmap = this->positions[i].flipped_bitmap;
         if (flipped_bitmap != UINT32_MAX)
            roaring::api::roaring_bitmap_andnot_inplace(&this->positions[i].bitmaps[flipped_bitmap].roaring, &current_gap_extension_candidates.roaring);
         for (unsigned gap_pos : current_gap_extension_candidates) {
            this->start_gaps[gap_pos]++;
         }
      }
   }
   {
      Roaring current_gap_extension_candidates;
      current_gap_extension_candidates.addRange(0, seq_store.sequence_count);
      for (unsigned i = genomeLength - 1; i >= genomeLength - genomeLength / 4; i--) {
         current_gap_extension_candidates &= seq_store.positions[i].bitmaps[Symbol::gap];
         roaring::api::roaring_bitmap_andnot_inplace(&this->positions[i].bitmaps[Symbol::gap].roaring, &current_gap_extension_candidates.roaring);
         uint32_t flipped_bitmap = this->positions[i].flipped_bitmap;
         if (flipped_bitmap != UINT32_MAX)
            roaring::api::roaring_bitmap_andnot_inplace(&this->positions[i].bitmaps[flipped_bitmap].roaring, &current_gap_extension_candidates.roaring);
         for (unsigned gap_pos : current_gap_extension_candidates) {
            this->end_gaps[gap_pos]++;
         }
      }
   }
}

SequenceStore::SequenceStore(const CompressedSequenceStore& c_seq_store) {
   using roaring::Roaring;

   this->sequence_count = c_seq_store.sequence_count;

   for (unsigned i = 0; i < genomeLength; ++i) {
      positions[i].flipped_bitmap = c_seq_store.positions[i].flipped_bitmap;
      for (uint16_t s = 0; s < symbolCount; s++) {
         positions[i].bitmaps[s] = c_seq_store.positions[i].bitmaps[s];
      }
   }
   {
      std::vector<std::vector<uint32_t>> gaps_per_pos(genomeLength / 4);
      for (unsigned seq_id = 0; seq_id < sequence_count; ++seq_id) {
         for (unsigned pos = 0; pos < c_seq_store.start_gaps[seq_id]; ++pos) {
            gaps_per_pos[pos].push_back(seq_id);
         }
      }
      for (unsigned i = 0; i < genomeLength / 4; i++) {
         const Roaring tmp(gaps_per_pos[i].size(), gaps_per_pos[i].data());
         roaring::api::roaring_bitmap_or_inplace(&this->positions[i].bitmaps[Symbol::gap].roaring, &tmp.roaring);
         uint32_t flipped_bitmap = this->positions[i].flipped_bitmap;
         if (flipped_bitmap != UINT32_MAX)
            roaring::api::roaring_bitmap_or_inplace(&this->positions[i].bitmaps[flipped_bitmap].roaring, &tmp.roaring);
      }
   }
   {
      /// for every position where end gaps are possible, calculate the vector of all IDs
      std::vector<std::vector<uint32_t>> gaps_per_pos(genomeLength / 4);
      for (unsigned seq_id = 0; seq_id < sequence_count; ++seq_id) {
         for (unsigned pos = 0; pos < c_seq_store.end_gaps[seq_id]; ++pos) {
            gaps_per_pos[pos].push_back(seq_id);
         }
      }
      for (unsigned i = genomeLength - 1; i >= genomeLength - genomeLength / 4; i--) {
         const Roaring tmp(gaps_per_pos[genomeLength - i - 1].size(), gaps_per_pos[genomeLength - i - 1].data());
         roaring::api::roaring_bitmap_or_inplace(&this->positions[i].bitmaps[Symbol::gap].roaring, &tmp.roaring);
         uint32_t flipped_bitmap = this->positions[i].flipped_bitmap;
         if (flipped_bitmap != UINT32_MAX)
            roaring::api::roaring_bitmap_or_inplace(&this->positions[i].bitmaps[flipped_bitmap].roaring, &tmp.roaring);
      }
   }
}
