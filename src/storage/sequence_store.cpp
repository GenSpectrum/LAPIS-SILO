//
// Created by Alexander Taepper on 01.09.22.
//

#include <syncstream>
#include <silo/storage/sequence_store.h>
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>

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
   auto tmp = computeSize();
   std::osyncstream(io) << "partition size (portable): " << number_fmt(tmp.first) << std::endl;
   std::osyncstream(io) << "partition size: " << number_fmt(tmp.second) << std::endl;
   return 0;
}

void SequenceStore::interpret_offset_p(const std::vector<std::string>& genomes, uint32_t offset) {
   tbb::blocked_range<unsigned> range(0, genomeLength, genomeLength / 64);
   tbb::parallel_for(range, [&](const decltype(range)& local) {
      std::vector<std::vector<unsigned>> symbolPositions(symbolCount);
      for (unsigned col = local.begin(); col != local.end(); ++col) {
         for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
            char c = genomes[index2][col];
            Symbol s = to_symbol(c);
            symbolPositions[s].push_back(offset + index2);
         }
         for (unsigned symbol = 0; symbol != symbolCount; ++symbol)
            if (!symbolPositions[symbol].empty()) {
               this->positions[col].bitmaps[symbol].addMany(symbolPositions[symbol].size(), symbolPositions[symbol].data());
               symbolPositions[symbol].clear();
            }
      }
   });
   this->sequence_count += genomes.size();
}

/// Appends the sequences in genome to the current bitmaps in SequenceStore and increases sequenceCount
void SequenceStore::interpret(const std::vector<std::string>& genomes) {
   // Putting sequences to the end is the same as offsetting them to sequence_count
   interpret_offset_p(genomes, this->sequence_count);
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
