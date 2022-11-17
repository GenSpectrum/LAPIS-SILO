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

int silo::db_info(const SequenceStore& db, std::ostream& io) {
   io << "sequence count: " << number_fmt(db.sequenceCount) << std::endl;
   io << "total size: " << number_fmt(db.computeSize()) << std::endl;
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
   db_info(db, io);
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

static void interpret_offset_p(SequenceStore& db, const std::vector<std::string>& genomes, uint32_t offset) {
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
               db.positions[col].bitmaps[symbol].addMany(symbolPositions[symbol].size(), symbolPositions[symbol].data());
               symbolPositions[symbol].clear();
            }
      }
   });
   db.sequenceCount += genomes.size();
}

/// Appends the sequences in genome to the current bitmaps in SequenceStore and increases sequenceCount
static void interpret(SequenceStore& db, const std::vector<std::string>& genomes) {
   // Putting sequences to the end is the same as offsetting them to sequence_count
   interpret_offset_p(db, genomes, db.sequenceCount);
}

void silo::process_raw(SequenceStore& db, std::istream& in) {
   static constexpr unsigned interpretSize = 1024;

   std::vector<std::string> genomes;
   while (true) {
      std::string epi_isl, genome;
      if (!getline(in, epi_isl) || epi_isl.empty()) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         std::cerr << "length mismatch!" << std::endl;
         return;
      }
      genomes.push_back(std::move(genome));
      if (genomes.size() >= interpretSize) {
         interpret(db, genomes);
         genomes.clear();
      }
   }
   interpret(db, genomes);
   db_info(db, std::cout);
}

void silo::process(SequenceStore& db, std::istream& in) {
   static constexpr unsigned interpretSize = 1024;

   uint32_t sid_ctr = db.sequenceCount;
   std::vector<std::string> genomes;
   while (true) {
      std::string epi_isl, genome;
      if (!getline(in, epi_isl) || epi_isl.empty()) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         std::cerr << "length mismatch!" << std::endl;
         return;
      }
      uint64_t epi = stoi(epi_isl.substr(9));

      genomes.push_back(std::move(genome));
      if (genomes.size() >= interpretSize) {
         interpret(db, genomes);
         genomes.clear();
      }

      uint32_t sid = sid_ctr++;
      db.epi_to_sid[epi] = sid;
      db.sid_to_epi.push_back(epi);
   }
   interpret(db, genomes);
   db_info(db, std::cout);
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

void silo::process_chunked_on_the_fly(SequenceStore& sdb, MetaStore& mdb, std::istream& in) {
   std::cout << "Now calculating partition offsets" << std::endl;

   std::vector<uint32_t> chunk_to_offset(mdb.pid_count);
   std::vector<uint32_t> chunk_to_realcount(mdb.pid_count);

   while (true) {
      std::string epi_isl;
      if (!getline(in, epi_isl)) break;
      in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // Add the count to the respective part
      uint64_t epi = stoi(epi_isl.substr(9));

      if (!mdb.epi_to_pid.contains(epi)) {
         // TODO logging
         continue;
      }

      uint16_t pid = mdb.epi_to_pid[epi];
      auto part = mdb.pid_to_chunk[pid];
      ++chunk_to_realcount[part];
   }

   // Escalate offsets from start to finish
   uint32_t cumulative_offset = 0;
   for (int i = 0; i < mdb.pid_count; ++i) {
      chunk_to_offset[i] += cumulative_offset;
      cumulative_offset += chunk_to_realcount[i];
   }

   // cumulative_offset should be equal to sequence count now

   std::cout << "Finished calculating chunk offsets." << std::endl;

   static constexpr unsigned interpretSize = 1024;

   // these offsets lag by chunk.
   std::vector<uint32_t> dynamic_offsets(chunk_to_offset);
   // actually these are the same offsets just without lagging by chunk.
   std::vector<uint32_t> sid_ctrs(chunk_to_offset);
   std::vector<std::vector<std::string>> pid_to_genomes;
   pid_to_genomes.resize(mdb.pid_count + 1);
   while (true) {
      std::string epi_isl, genome;
      if (!getline(in, epi_isl)) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         std::cerr << "length mismatch!" << std::endl;
         return;
      }
      uint64_t epi = stoi(epi_isl.substr(9));

      if (!mdb.epi_to_pid.contains(epi)) {
         // TODO logging
         continue;
      }

      uint16_t pid = mdb.epi_to_pid.at(epi);
      auto part = mdb.pid_to_chunk[pid];
      ++chunk_to_realcount[part];

      auto& genomes = pid_to_genomes[pid];
      genomes.emplace_back(std::move(genome));
      if (genomes.size() >= interpretSize) {
         interpret_offset_p(sdb, genomes, dynamic_offsets[pid]);
         dynamic_offsets[pid] += genomes.size();
         genomes.clear();
      }

      uint32_t sid = sid_ctrs[pid]++;
      sdb.epi_to_sid[epi] = sid;
   }

   for (uint16_t pid = 0; pid < mdb.pid_count + 1; ++pid) {
      interpret_offset_p(sdb, pid_to_genomes[pid], dynamic_offsets[pid]);
   }

   // now also calculate the reverse direction for the epi<->sid relationship.
   sdb.sid_to_epi.resize(sdb.epi_to_sid.size());
   for (auto& x : sdb.epi_to_sid) {
      sdb.sid_to_epi[x.second] = x.first;
   }

   db_info(sdb, std::cout);
}

// Only for testing purposes. Very inefficient. Will insert the genome in specific positions to the sequenceStore
void interpret_specific(SequenceStore& db, const std::vector<std::pair<uint64_t, std::string>>& genomes) {
   std::vector<unsigned> offsets[symbolCount];
   for (unsigned index = 0; index != genomeLength; ++index) {
      for (const auto& idx_genome : genomes) {
         char c = idx_genome.second[index];
         Symbol s = to_symbol(c);
         offsets[s].push_back(idx_genome.first);
      }
      for (unsigned index2 = 0; index2 != symbolCount; ++index2) {
         if (!offsets[index2].empty()) {
            db.positions[index].bitmaps[index2].addMany(offsets[index2].size(), offsets[index2].data());
            offsets[index2].clear();
         }
      }
   }
   db.sequenceCount += genomes.size();
}
