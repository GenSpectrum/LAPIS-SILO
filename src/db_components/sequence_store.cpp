//
// Created by Alexander Taepper on 01.09.22.
//
#include "silo/db_components/sequence_store.h"
#include <tbb/blocked_range.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/parallel_for.h>

using namespace silo;
using ios = std::ios;

roaring::Roaring SequenceStore::bma(size_t pos, Residue r) const {
   switch (r) {
      case aA: {
         const roaring::Roaring* tmp[8] = {bm(pos, A), bm(pos, N),
                                           bm(pos, R), bm(pos, W), bm(pos, M),
                                           bm(pos, D), bm(pos, H), bm(pos, V)};
         return roaring::Roaring::fastunion(8, tmp);
      }
      case aC: {
         const roaring::Roaring* tmp[8] = {bm(pos, C), bm(pos, N),
                                           bm(pos, Y), bm(pos, S), bm(pos, M),
                                           bm(pos, B), bm(pos, H), bm(pos, V)};
         return roaring::Roaring::fastunion(8, tmp);
      }
      case aG: {
         const roaring::Roaring* tmp[8] = {bm(pos, G), bm(pos, N),
                                           bm(pos, R), bm(pos, S), bm(pos, K),
                                           bm(pos, D), bm(pos, B), bm(pos, V)};
         return roaring::Roaring::fastunion(8, tmp);
      }
      case aT: {
         const roaring::Roaring* tmp[8] = {bm(pos, T), bm(pos, N),
                                           bm(pos, Y), bm(pos, W), bm(pos, K),
                                           bm(pos, D), bm(pos, H), bm(pos, B)};
         return roaring::Roaring::fastunion(8, tmp);
      }
   }
   std::cerr << "Should not happen, number of residue changed?" << std::endl;
   return roaring::Roaring{};
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
