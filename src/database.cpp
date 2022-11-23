//
// Created by Alexander Taepper on 16.11.22.
//

#include <syncstream>
#include <silo/database.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>

void silo::Database::build(const std::string& part_prefix, const std::string& meta_suffix, const std::string& seq_suffix) {
   partitions.resize(part_def->partitions.size());
   tbb::blocked_range<size_t> r(0, part_def->partitions.size());
   tbb::parallel_for(r, [&](const decltype(r)& subr) {
      for (size_t i = subr.begin(), limit = subr.end(); i != limit; ++i) {
         const auto& part = part_def->partitions[i];
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
            unsigned count2 = processMeta(partitions[i].meta_store, meta_in, alias_key);
            if (count1 != count2) {
               // Fatal error
               std::cerr << "Sequences in meta data and sequence data for chunk " << chunk_string(i, j) << " are not equal." << std::endl;
               std::cerr << "Abort build." << std::endl;
               partitions.clear();
               return;
            }
            partitions[i].sequenceCount += count1;
         }
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

void silo::Database::analyse() {
   /* std::vector<std::vector<unsigned>> counts_per_pos_per_symbol;
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
   } */
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
   r_stat s_total{};
   uint64_t total_size_comp = 0;
   uint64_t total_size_frozen = 0;
   /// Because the counters in r_stat are 32 bit and overflow..
   uint64_t n_bytes_array_containers; /* number of allocated bytes in array
                                           containers */
   uint64_t n_bytes_run_containers; /* number of allocated bytes in run
                                           containers */
   uint64_t n_bytes_bitset_containers; /* number of allocated bytes in  bitmap
                                           containers */

   tbb::parallel_for_each(partitions.begin(), partitions.end(), [&](const DatabasePartition& dbp) {
      std::vector<uint32_t> bitset_containers_by_500pos_local((genomeLength / 500) + 1);
      r_stat s_local{};
      uint64_t total_size_comp_local = 0;
      uint64_t total_size_frozen_local = 0;
      uint64_t n_bytes_array_containers_local = 0;
      uint64_t n_bytes_run_containers_local = 0;
      uint64_t n_bytes_bitset_containers_local = 0;
      {
         r_stat s;
         for (unsigned pos = 0; pos < genomeLength; ++pos) {
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
                  lock.lock();
                  bitset_containers_by_500pos_local[pos / 500] += s.n_bitset_containers;
                  lock.unlock();
               }
            }
         }
      }
      lock.lock();
      for (unsigned i = 0; i < bitset_containers_by_500pos.size(); ++i) {
         bitset_containers_by_500pos[i] += bitset_containers_by_500pos_local[i];
      }
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

   io << "Bitmap distribution by position " << std::endl;
   for (unsigned i = 0; i < (genomeLength / 500) + 1; ++i) {
      uint32_t bitmaps_at_pos = bitset_containers_by_500pos[i];
      io << "Pos: [" << i * 500 << "," << ((i + 1) * 500) << "): " << bitmaps_at_pos << '\n';
   }
   io.flush();

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

unsigned silo::processMeta(MetaStore& mdb, std::istream& in, const std::unordered_map<std::string, std::string>& alias_key) {
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

      silo::inputSequenceMeta(mdb, epi, pango_lineage, date, region, country, division);
      ++sequence_count;
   }

   return sequence_count;
}
