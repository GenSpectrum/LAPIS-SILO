//
// Created by Alexander Taepper on 01.09.22.
//
#include "silo/sequence_store.h"

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

static inline void addStat(r_stat r1, r_stat r2){
   r1.cardinality += r2.cardinality;
   if(r2.max_value > r1.max_value) r1.max_value = r2.max_value;
   if(r2.min_value < r1.min_value) r1.min_value = r2.min_value;
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

   r_stat s_total;
   r_stat s;
   for(const Position& p : db.positions){
      for(const Roaring& bm : p.bitmaps){
         roaring_bitmap_statistics(&db.positions[0].bitmaps[0].roaring, &s);
         addStat(s_total, s);
      }
   }
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
      ::boost::archive::binary_iarchive ia(ifs);
      // read class state from archive
      ia >> db;
      // archive and stream closed when destructors are called
   }
   return 0;
}

static void interpret_offset(SequenceStore& db, const std::vector<std::string>& genomes, uint32_t offset) {
   std::vector<unsigned> offsets[symbolCount];
   for (unsigned index = 0; index != genomeLength; ++index) {
      for (unsigned index2 = 0, limit2 = genomes.size(); index2 != limit2; ++index2) {
         char c = genomes[index2][index];
         Symbol s = to_symbol(c);
         offsets[s].push_back(offset + index2);
      }
      for (unsigned index2 = 0; index2 != symbolCount; ++index2)
         if (!offsets[index2].empty()) {
            db.positions[index].bitmaps[index2].addMany(offsets[index2].size(), offsets[index2].data());
            offsets[index2].clear();
         }
   }
   db.sequenceCount += genomes.size();
}

/// Appends the sequences in genome to the current bitmaps in SequenceStore and increases sequenceCount
static void interpret(SequenceStore& db, const std::vector<std::string>& genomes) {
   // Putting sequences to the end is the same as offsetting them to sequence_count
   interpret_offset(db, genomes, db.sequenceCount);
}

void silo::process_raw(SequenceStore& db, std::istream& in) {
   static constexpr unsigned chunkSize = 1024;

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
      if (genomes.size() >= chunkSize) {
         interpret(db, genomes);
         genomes.clear();
      }
   }
   interpret(db, genomes);
   db_info(db, std::cout);
}

void silo::process(SequenceStore& db, MetaStore& mdb, std::istream& in) {
   static constexpr unsigned chunkSize = 1024;

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
      if (genomes.size() >= chunkSize) {
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
   unsigned count_true = 0;
   for (Position& p : db.positions) {
      for (auto& bm : p.bitmaps) {
         if (bm.runOptimize()) ++count_true;
      }
   }
   return count_true;
}

void silo::calc_partition_offsets(SequenceStore& db, MetaStore& mdb, std::istream& in) {
   std::cout << "Now calculating partition offsets" << std::endl;

   // Clear the vector and resize
   // TODO for future proofing, instead of clearing, extending them?
   db.part_to_offset.clear();
   db.part_to_offset.resize(mdb.pid_count);
   db.part_to_realcount.clear();
   db.part_to_realcount.resize(mdb.pid_count);

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
      auto part = mdb.pid_to_partition[pid];
      ++db.part_to_realcount[part];
   }

   // Escalate offsets from start to finish
   uint32_t cumulative_offset = 0;
   for (int i = 0; i < mdb.pid_count; ++i) {
      db.part_to_offset[i] += cumulative_offset;
      cumulative_offset += db.part_to_realcount[i];
   }

   // cumulative_offset should be equal to sequence count now

   std::cout << "Finished calculating partition offsets." << std::endl;
}

// TODO  this clears the SequenceStore? doesn't have to be
//       see also calc_partition_offsets, maybe condense into one function?
//       Does not really make much senese to call them independently
void silo::process_partitioned_on_the_fly(SequenceStore& db, MetaStore& mdb, std::istream& in) {
   static constexpr unsigned chunkSize = 1024;

   // these offsets lag by chunk.
   std::vector<uint32_t> dynamic_offsets(db.part_to_offset);
   // actually these are the same offsets just without lagging by chunk.
   std::vector<uint32_t> sid_ctrs(db.part_to_offset);
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
      auto part = mdb.pid_to_partition[pid];
      ++db.part_to_realcount[part];

      auto& genomes = pid_to_genomes[pid];
      genomes.emplace_back(std::move(genome));
      if (genomes.size() >= chunkSize) {
         interpret_offset(db, genomes, dynamic_offsets[pid]);
         dynamic_offsets[pid] += genomes.size();
         genomes.clear();
      }

      uint32_t sid = sid_ctrs[pid]++;
      db.epi_to_sid[epi] = sid;
   }

   for (uint16_t pid = 0; pid < mdb.pid_count + 1; ++pid) {
      interpret_offset(db, pid_to_genomes[pid], dynamic_offsets[pid]);
   }

   // now also calculate the reverse direction for the epi<->sid relationship.
   db.sid_to_epi.resize(db.epi_to_sid.size());
   for (auto& x : db.epi_to_sid) {
      db.sid_to_epi[x.second] = x.first;
   }

   db_info(db, std::cout);
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

void silo::partition_sequences(MetaStore& mdb, std::istream& in, const std::string& output_prefix_) {
   std::cout << "Now partitioning fasta file to " << output_prefix_ << std::endl;
   std::vector<std::unique_ptr<std::ostream>> part_to_ostream;
   const std::string output_prefix = output_prefix_ + '_';
   for (unsigned part = 0; part < mdb.partitions.size(); ++part) {
      auto out = make_unique<std::ofstream>(output_prefix + std::to_string(part) + ".fasta");
      part_to_ostream.emplace_back(std::move(out));
   }
   std::cout << "Created file streams for  " << output_prefix_ << std::endl;
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

      auto pid = mdb.epi_to_pid.at(epi);
      auto part = mdb.pid_to_partition[pid];
      *part_to_ostream[part] << epi_isl << std::endl
                             << genome << std::endl;
   }
   std::cout << "Finished partitioning to " << output_prefix_ << std::endl;
}

void silo::sort_partitions(const MetaStore& mdb, const std::string& output_prefix_) {
   const std::string output_prefix = output_prefix_ + '_';

   unsigned n = mdb.partitions.size();
   // std::vector<std::thread> threads(n);
   for (unsigned part = 0; part < n; ++part) {
      const std::string& file_name = output_prefix + std::to_string(part);
      sort_partition(mdb, file_name, part, SortOption::bydate);
      // threads[n] = std::thread(sort_partition, mdb, file_name, part, SortOption::bydate);
   }

   /*
   for (unsigned part = 0; part < n; ++part) {
      threads[part].join();
   }
   */
}

void silo::sort_partition(const MetaStore& mdb, const std::string& file_name, unsigned part, SortOption option) {
   if (option != SortOption::bydate) {
      return;
   }
   silo::istream_wrapper in_wrap(file_name + ".fasta");
   std::istream& in = in_wrap.get_is();
   std::ofstream out(file_name + "_sorted.fasta");

   // Function does:
   // Read file once, fill all dates, sort dates,
   // calculated target position for every genome
   // Reset gpointer, read file again, putting every genome at the correct position.
   // Write file to ostream

   struct EPIDate {
      uint64_t epi;
      time_t date;
      uint32_t file_pos;
   };
   std::vector<EPIDate> firstRun;
   firstRun.reserve(mdb.partitions.at(part).count);

   uint32_t count = 0;
   while (true) {
      std::string epi_isl;
      if (!getline(in, epi_isl)) break;
      in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

      // Add the count to the respective pid
      uint64_t epi = stoi(epi_isl.substr(9));

      // Could assert that pid is the same as in meta_data?
      uint32_t sidm = mdb.epi_to_sidM.at(epi);
      time_t date = mdb.sidM_to_date[sidm];
      firstRun.emplace_back(EPIDate{epi, date, count++});
   }

   std::cout << "Finished first run for partition: " << part << std::endl;

   auto sorter = [](const EPIDate& s1, const EPIDate& s2) {
      return s1.date < s2.date;
   };
   std::sort(firstRun.begin(), firstRun.end(), sorter);

   std::cout << "Sorted first run for partition: " << part << std::endl;

   std::vector<uint32_t> file_pos_to_sorted_pos(count);
   unsigned count2 = 0;
   for (auto& x : firstRun) {
      file_pos_to_sorted_pos[x.file_pos] = count2++;
   }

   assert(count == count2);

   std::cout << "Calculated postitions for every sequence: " << part << std::endl;

   in.clear(); // clear fail and eof bits
   in.seekg(0, std::ios::beg); // back to the start!

   std::cout << "Reset file seek, now read second time, sorted: " << part << std::endl;

   std::vector<std::string> lines_sorted(2 * count);
   for (auto pos : file_pos_to_sorted_pos) {
      std::string epi_isl, genome;
      if (!getline(in, lines_sorted[2 * pos])) {
         std::cerr << "Reached EOF too early." << std::endl;
         return;
      }
      if (!getline(in, lines_sorted[2 * pos + 1])) {
         std::cerr << "Reached EOF too early." << std::endl;
         return;
      }
   }

   for (const std::string& line : lines_sorted) {
      out << line << '\n';
   }
}
