//
// Created by Alexander Taepper on 16.11.22.
//

#include <silo/common/fix_rh_map.hpp>
#include <syncstream>
#include <silo/common/SizeSketch.h>
#include <silo/common/hashing.h>
#include <silo/database.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>

void silo::Database::build(const std::string& part_prefix, const std::string& meta_suffix, const std::string& seq_suffix) {
   partitions.resize(part_def->partitions.size());
   tbb::parallel_for((size_t) 0, part_def->partitions.size(), [&](size_t i) {
      const auto& part = part_def->partitions[i];
      partitions[i].chunks = part.chunks;
      for (unsigned j = 0; j < part.chunks.size(); ++j) {
         std::string name;
         if (i > 0) { // TODO cleaner dealing with chunk to filename mapping..
            name = part_prefix + chunk_string(j, i);
         } else {
            name = part_prefix + chunk_string(i, j);
         }
         istream_wrapper seq_in(name + seq_suffix);
         std::ifstream meta_in(name + meta_suffix);
         std::osyncstream(std::cout) << "Extending sequence-store from input file: " << name << std::endl;
         unsigned count1 = processSeq(partitions[i].seq_store, seq_in.get_is());
         unsigned count2 = processMeta(partitions[i].meta_store, meta_in, alias_key, *dict);
         if (count1 != count2) {
            // Fatal error
            std::cerr << "Sequences in meta data and sequence data for chunk " << chunk_string(i, j) << " are not equal." << std::endl;
            std::cerr << "Abort build." << std::endl;
            partitions.clear();
            return;
         }
         partitions[i].sequenceCount += count1;
      }
   });
}

void silo::DatabasePartition::finalize() {
   std::vector<std::vector<unsigned>> counts_per_pos_per_symbol;
   counts_per_pos_per_symbol.resize(genomeLength);
   for (std::vector<unsigned>& v : counts_per_pos_per_symbol) {
      v.resize(symbolCount);
   }

   tbb::parallel_for((unsigned) 0, genomeLength, [&](unsigned p) {
      unsigned max_symbol = 0;
      unsigned max_count = seq_store.positions[p].bitmaps[0].cardinality();

      for (unsigned symbol = 1; symbol < symbolCount; ++symbol) {
         unsigned count = seq_store.positions[p].bitmaps[symbol].cardinality();
         if (count > max_count) {
            max_symbol = symbol;
            max_count = count;
         }
      }
      seq_store.positions[p].reference = max_symbol;
      seq_store.positions[p].bitmaps[max_symbol].flip(0, sequenceCount);
   });
}

void silo::Database::finalize() {
   tbb::parallel_for_each(partitions.begin(), partitions.end(), [&](DatabasePartition& p) {
      p.finalize();
   });
}

/*
void silo::Database::analyse() {
   std::vector<std::vector<unsigned>> counts_per_pos_per_symbol;
   counts_per_pos_per_symbol.resize(genomeLength);
   for (std::vector<unsigned>& v : counts_per_pos_per_symbol) {
      v.resize(symbolCount);
   }

   for (const DatabasePartition& dbp : partitions) {
      tbb::parallel_for((unsigned) 0, genomeLength, [&](unsigned p) {
         for (unsigned symbol = 0; symbol < symbolCount; ++symbol) {
            ++counts_per_pos_per_symbol[p][symbol];
         }
      });
   }
}*/

using r_stat = roaring::api::roaring_statistics_t;

static inline void addStat(r_stat& r1, const r_stat& r2) {
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

int silo::Database::db_info(std::ostream& io) {
   std::atomic<uint32_t> sequence_count = 0;
   std::atomic<uint64_t> total_size = 0;
   tbb::parallel_for_each(partitions.begin(), partitions.end(), [&](const DatabasePartition& dbp) {
      sequence_count += dbp.sequenceCount;
      total_size += dbp.seq_store.computeSize();
   });

   io << "sequence count: " << number_fmt(sequence_count) << std::endl;
   io << "total size: " << number_fmt(total_size) << std::endl;

   return 0;
}

int silo::Database::db_info_detailed(std::ostream& io) {
   std::vector<size_t> size_by_symbols(symbolCount);

   tbb::parallel_for((unsigned) 0, symbolCount, [&](unsigned symbol) {
      for (const DatabasePartition& dbp : partitions) {
         for (const auto& position : dbp.seq_store.positions) {
            size_by_symbols[symbol] += position.bitmaps[symbol].getSizeInBytes();
         }
      }
   });
   for (unsigned symbol = 0; symbol < symbolCount; ++symbol) {
      io << "size for symbol '" << symbol_rep[symbol] << "': "
         << number_fmt(size_by_symbols[symbol]) << std::endl;
   }

   std::mutex lock;
   std::vector<uint32_t> bitset_containers_by_500pos((genomeLength / 500) + 1);
   std::vector<uint32_t> gap_bitset_containers_by_500pos((genomeLength / 500) + 1);
   std::vector<uint32_t> N_bitset_containers_by_500pos((genomeLength / 500) + 1);
   r_stat s_total{};
   uint64_t total_size_comp = 0;
   uint64_t total_size_frozen = 0;
   /// Because the counters in r_stat are 32 bit and overflow..
   uint64_t n_bytes_array_containers = 0;
   uint64_t n_bytes_run_containers = 0;
   uint64_t n_bytes_bitset_containers = 0;

   tbb::parallel_for((unsigned) 0, genomeLength, [&](unsigned pos) {
      r_stat s_local{};
      uint64_t total_size_comp_local = 0;
      uint64_t total_size_frozen_local = 0;
      uint64_t n_bytes_array_containers_local = 0;
      uint64_t n_bytes_run_containers_local = 0;
      uint64_t n_bytes_bitset_containers_local = 0;
      {
         r_stat s;
         for (const auto& dbp : partitions) {
            const Position& p = dbp.seq_store.positions[pos];
            for (const Roaring& bm : p.bitmaps) {
               roaring_bitmap_statistics(&bm.roaring, &s);
               addStat(s_local, s);
               total_size_comp_local += bm.getSizeInBytes();
               total_size_frozen_local += bm.getFrozenSizeInBytes();
               n_bytes_array_containers_local += s.n_bytes_array_containers;
               n_bytes_run_containers_local += s.n_bytes_run_containers;
               n_bytes_bitset_containers_local += s.n_bytes_bitset_containers;
               if (s.n_bitset_containers > 0) {
                  if (pos == Symbol::N) {
                     N_bitset_containers_by_500pos[pos / 500] += s.n_bitset_containers;
                  } else if (pos == Symbol::gap) {
                     gap_bitset_containers_by_500pos[pos / 500] += s.n_bitset_containers;
                  } else {
                     bitset_containers_by_500pos[pos / 500] += s.n_bitset_containers;
                  }
               }
            }
         }
      }
      lock.lock();
      addStat(s_total, s_local);
      total_size_comp += total_size_comp_local;
      total_size_frozen += total_size_frozen_local;
      n_bytes_array_containers += n_bytes_array_containers_local;
      n_bytes_run_containers += n_bytes_run_containers_local;
      n_bytes_bitset_containers += n_bytes_bitset_containers_local;
      lock.unlock();
   });
   io << "Total bitmap containers " << number_fmt(s_total.n_containers) << ", of those there are " << std::endl
      << "array: " << number_fmt(s_total.n_array_containers) << std::endl
      << "run: " << number_fmt(s_total.n_run_containers) << std::endl
      << "bitset: " << number_fmt(s_total.n_bitset_containers) << std::endl;
   io << "Total bitmap values " << number_fmt(s_total.cardinality) << ", of those there are " << std::endl
      << "array: " << number_fmt(s_total.n_values_array_containers) << std::endl
      << "run: " << number_fmt(s_total.n_values_run_containers) << std::endl
      << "bitset: " << number_fmt(s_total.n_values_bitset_containers) << std::endl;
   uint64_t total_size = n_bytes_array_containers + n_bytes_run_containers + n_bytes_bitset_containers;

   io << "Total bitmap byte size " << number_fmt(total_size_frozen) << " (frozen) " << std::endl;
   io << "Total bitmap byte size " << number_fmt(total_size_comp) << " (compute_size) " << std::endl;
   io << "Total bitmap byte size " << number_fmt(total_size) << ", of those there are " << std::endl
      << "array: " << number_fmt(s_total.n_bytes_array_containers) << std::endl
      << "run: " << number_fmt(s_total.n_bytes_run_containers) << std::endl
      << "bitset: " << number_fmt(s_total.n_bytes_bitset_containers) << std::endl;

   io << "Bitmap distribution by position #NON_GAP (#GAP)" << std::endl;
   for (unsigned i = 0; i < (genomeLength / 500) + 1; ++i) {
      uint32_t gap_bitsets_at_pos = gap_bitset_containers_by_500pos[i];
      uint32_t N_bitmaps_at_pos = N_bitset_containers_by_500pos[i];
      uint32_t bitmaps_at_pos = bitset_containers_by_500pos[i];
      io << "Pos: [" << i * 500 << "," << ((i + 1) * 500) << "): " << bitmaps_at_pos << " (N: " << N_bitmaps_at_pos << ", -: " << gap_bitsets_at_pos << ")" << '\n';
   }
   io.flush();

   io << "Partition reference genomes: " << std::endl;
   for (const DatabasePartition& dbp : partitions) {
      for (const Position& pos : dbp.seq_store.positions) {
         io << symbol_rep[pos.reference];
      }
      io << std::endl;
   }

   return 0;
}

unsigned silo::processSeq(silo::SequenceStore& seq_store, std::istream& in) {
   static constexpr unsigned interpretSize = 1024;

   unsigned sequence_count = 0;

   std::vector<std::string> genomes;
   while (true) {
      std::string epi_isl, genome;
      if (!getline(in, epi_isl)) break;
      if (!getline(in, genome)) break;
      if (genome.length() != genomeLength) {
         std::cerr << "length mismatch!" << std::endl;
         throw std::runtime_error("length mismatch.");
      }

      genomes.push_back(std::move(genome));
      if (genomes.size() >= interpretSize) {
         seq_store.interpret(genomes);
         genomes.clear();
      }

      ++sequence_count;
   }
   seq_store.interpret(genomes);
   seq_store.db_info(std::cout);

   return sequence_count;
}

unsigned silo::processMeta(MetaStore& mdb, std::istream& in, const std::unordered_map<std::string, std::string>& alias_key, const Dictionary& dict) {
   // Ignore header line.
   in.ignore(LONG_MAX, '\n');

   unsigned sequence_count = 0;

   while (true) {
      std::string epi_isl, pango_lineage_raw, date, region, country, division;
      if (!getline(in, epi_isl, '\t')) break;
      if (!getline(in, pango_lineage_raw, '\t')) break;
      if (!getline(in, date, '\t')) break;
      if (!getline(in, region, '\t')) break;
      if (!getline(in, country, '\t')) break;
      if (!getline(in, division, '\n')) break;

      /// Deal with pango_lineage alias:
      std::string pango_lineage = resolve_alias(alias_key, pango_lineage_raw);

      std::string tmp = epi_isl.substr(8);
      uint64_t epi = stoi(tmp);

      struct std::tm tm {};
      std::istringstream ss(date);
      ss >> std::get_time(&tm, "%Y-%m-%d");
      std::time_t time = mktime(&tm);

      std::vector<uint64_t> extra_cols;
      extra_cols.push_back(dict.get_id(division));

      silo::inputSequenceMeta(mdb, epi, time, dict.get_pangoid(pango_lineage),
                              dict.get_regionid(region), dict.get_countryid(country), extra_cols);
      ++sequence_count;
   }

   return sequence_count;
}

void silo::save_pango_defs(const silo::pango_descriptor_t& pd, std::ostream& out) {
   for (auto& x : pd.pangos) {
      out << x.pango_lineage << '\t' << x.count << '\n';
   }
   out.flush();
}

silo::pango_descriptor_t silo::load_pango_defs(std::istream& in) {
   silo::pango_descriptor_t descriptor;
   std::string lineage, count_str;
   uint32_t count;
   while (in && !in.eof()) {
      if (!getline(in, lineage, '\t')) break;
      if (!getline(in, count_str, '\n')) break;
      count = atoi(count_str.c_str());
      descriptor.pangos.emplace_back(silo::pango_t{lineage, count});
   }
   return descriptor;
}

void silo::save_partitioning_descriptor(const silo::partitioning_descriptor_t& pd, std::ostream& out) {
   for (auto& part : pd.partitions) {
      out << "P\t" << part.name << '\t' << part.chunks.size() << '\t' << part.count << '\n';
      for (auto& chunk : part.chunks) {
         out << "C\t" << chunk.prefix << '\t' << chunk.pangos.size() << '\t' << chunk.count << '\t' << chunk.offset << '\n';
         for (auto& pango : chunk.pangos) {
            out << "L\t" << pango << '\n';
         }
      }
   }
}

void silo::Database::save(const std::string& save_dir) {
   if (!part_def) {
      std::cerr << "Cannot save db without part_def." << std::endl;
      return;
   }

   if (pango_def) {
      std::ofstream pango_def_file(save_dir + "pango_def.txt");
      if (!pango_def_file) {
         std::cerr << "Cannot open pango_def output file: " << (save_dir + "pango_def.txt") << std::endl;
         return;
      }
      save_pango_defs(*pango_def, pango_def_file);
   }
   {
      std::ofstream part_def_file(save_dir + "part_def.txt");
      if (!part_def_file) {
         std::cerr << "Cannot open part_def output file: " << (save_dir + "part_def.txt") << std::endl;
         return;
      }
      save_partitioning_descriptor(*part_def, part_def_file);
   }

   std::vector<std::ofstream> file_vec;
   for (unsigned i = 0; i < part_def->partitions.size(); ++i) {
      file_vec.push_back(std::ofstream(save_dir + 'P' + std::to_string(i) + ".silo"));

      if (!file_vec.back()) {
         std::cerr << "Cannot open partition output file for saving: " << (save_dir + 'P' + std::to_string(i) + ".silo") << std::endl;
         return;
      }
   }

   tbb::parallel_for((size_t) 0, part_def->partitions.size(), [&](size_t i) {
      ::boost::archive::binary_oarchive oa(file_vec[i]);
      oa << partitions[i];
   });
}

void silo::Database::load(const std::string& save_dir) {
   std::ifstream part_def_file(save_dir + "part_def.txt");
   if (!part_def_file) {
      std::cerr << "Cannot open part_def input file for loading: " << (save_dir + "part_def.txt") << std::endl;
      return;
   }
   part_def = std::make_unique<partitioning_descriptor_t>(load_partitioning_descriptor(part_def_file));

   std::ifstream pango_def_file(save_dir + "pango_def.txt");
   if (pango_def_file) {
      pango_def = std::make_unique<pango_descriptor_t>(load_pango_defs(pango_def_file));
   }

   std::vector<std::ifstream> file_vec;
   for (unsigned i = 0; i < part_def->partitions.size(); ++i) {
      file_vec.push_back(std::ifstream(save_dir + 'P' + std::to_string(i) + ".silo"));

      if (!file_vec.back()) {
         std::cerr << "Cannot open partition input file for loading: " << (save_dir + 'P' + std::to_string(i) + ".silo") << std::endl;
         return;
      }
   }

   partitions.resize(part_def->partitions.size());
   tbb::parallel_for((size_t) 0, part_def->partitions.size(), [&](size_t i) {
      ::boost::archive::binary_iarchive ia(file_vec[i]);
      ia >> partitions[i];
   });
}