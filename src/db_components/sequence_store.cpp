//
// Created by Alexander Taepper on 01.09.22.
//
#include "silo/db_components/sequence_store.h"
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
         throw std::runtime_error("Approximate query only on A C G T allowed.");
      }
   }
}

int SequenceStore::db_info(std::ostream& io) const {
   io << "sequence count: " << number_fmt(this->sequence_count) << std::endl;
   io << "total size: " << number_fmt(this->computeSize()) << std::endl;
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

[[maybe_unused]] unsigned silo::runoptimize(SequenceStore& db) {
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
