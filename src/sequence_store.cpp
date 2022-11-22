//
// Created by Alexander Taepper on 01.09.22.
//
#include "silo/sequence_store.h"
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

using r_stat = roaring::api::roaring_statistics_t;

static inline void addStat(r_stat& r1, r_stat& r2) {
   r1.cardinality += r2.cardinality;
   if (r2.max_value > r1.max_value) r1.max_value = r2.max_value;
   if (r2.min_value < r1.min_value) r1.min_value = r2.min_value;
   r1.n_array_containers += r2.n_array_containers;
   r1.n_run_containers += r2.n_run_containers;
   r1.n_bitset_containers += r2.n_bitset_containers;
   r1.n_bytes_array_containers += r2.n_bytes_array_containers;
   r1.n_bytes_run_containers += r2.n_bytes_run_containers;
   r1.n_bytes_bitset_containers += r2.n_bytes_bitset_containers;
   r1.n_values_array_containers += r2.n_values_array_containers;
   r1.n_values_run_containers += r2.n_values_run_containers;
   r1.n_values_bitset_containers += r2.n_values_bitset_containers;
   r1.n_containers += r2.n_containers;
   r1.sum_value += r2.sum_value;
}

int silo::db_info_detailed(const SequenceStore& db, std::ostream& io) {
   db.db_info(io);
   std::vector<size_t> size_by_symbols;
   size_by_symbols.resize(symbolCount);
   for (const auto& position : db.positions) {
      for (unsigned symbol = 0; symbol < symbolCount; ++symbol) {
         size_by_symbols[symbol] += position.bitmaps[symbol].getSizeInBytes();
      }
   }
   for (unsigned symbol = 0; symbol < symbolCount; ++symbol) {
      io << "size for symbol '" << symbol_rep[symbol] << "': "
         << number_fmt(size_by_symbols[symbol]) << std::endl;
   }

   r_stat s_total{};
   {
      r_stat s;
      for (const Position& p : db.positions) {
         for (const Roaring& bm : p.bitmaps) {
            roaring_bitmap_statistics(&bm.roaring, &s);
            addStat(s_total, s);
         }
      }
   }
   io << "Total bitmap containers " << number_fmt(s_total.n_containers) << ", of those there are " << std::endl
      << "array: " << number_fmt(s_total.n_array_containers) << std::endl
      << "run: " << number_fmt(s_total.n_run_containers) << std::endl
      << "bitset: " << number_fmt(s_total.n_bitset_containers) << std::endl;
   io << "Total bitmap values " << number_fmt(s_total.cardinality) << ", of those there are " << std::endl
      << "array: " << number_fmt(s_total.n_values_array_containers) << std::endl
      << "run: " << number_fmt(s_total.n_values_run_containers) << std::endl
      << "bitset: " << number_fmt(s_total.n_values_bitset_containers) << std::endl;
   io << "Total bitmap byte size " << number_fmt(db.computeSize()) << ", of those there are " << std::endl
      << "array: " << number_fmt(s_total.n_bytes_array_containers) << std::endl
      << "run: " << number_fmt(s_total.n_bytes_run_containers) << std::endl
      << "bitset: " << number_fmt(s_total.n_bytes_bitset_containers) << std::endl;

   return 0;
}

unsigned silo::save_db(const SequenceStore& db, const std::string& db_filename) {
   std::cout << "Writing out db." << std::endl;

   std::ofstream wf(db_filename, ios::out | ios::binary);
   if (!wf) {
      std::cerr << "Cannot open ofile: " << db_filename << std::endl;
      return 1;
   }

   {
      ::boost::archive::binary_oarchive oa(wf);
      // write class instance to archive
      oa << db;
      // archive and stream closed when destructors are called
   }

   return 0;
}

unsigned silo::load_db(SequenceStore& db, const std::string& db_filename) {
   {
      // create and open an archive for input
      std::ifstream ifs(db_filename, ios::binary);

      if (!ifs) {
         std::cerr << db_filename << " not found." << std::endl;
         return -1;
      }
      ::boost::archive::binary_iarchive ia(ifs);
      // read class state from archive
      ia >> db;
      // archive and stream closed when destructors are called
   }
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

unsigned silo::runoptimize(SequenceStore& db) {
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
