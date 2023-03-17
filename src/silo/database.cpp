#include <silo/common/input_stream_wrapper.h>
#include <silo/database.h>
#include <silo/prepare_dataset.h>
#include <silo/preprocessing/preprocessing_config.h>
#include <silo/preprocessing/preprocessing_exception.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>

#include <silo/common/silo_symbols.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <external/PerfEvent.hpp>
#include <filesystem>
#include <syncstream>

const std::string REFERENCE_GENOME_FILENAME = "reference_genome.txt";
const std::string PANGO_ALIAS_FILENAME = "pango_alias.txt";

std::vector<std::string> initGlobalReference(const std::string& working_directory) {
   std::filesystem::path const reference_genome_path(working_directory + REFERENCE_GENOME_FILENAME);
   if (!std::filesystem::exists(reference_genome_path)) {
      throw std::filesystem::filesystem_error(
         "Global reference genome file " + reference_genome_path.relative_path().string() +
            " does not exist",
         std::error_code()
      );
   }

   std::ifstream reference_file(reference_genome_path);
   std::vector<std::string> global_reference;
   while (true) {
      std::string line;
      if (!getline(reference_file, line, '\n')) {
         break;
      }
      if (line.find('N') != std::string::npos) {
         throw std::runtime_error("No N in reference genome allowed.");
      }
      global_reference.push_back(line);
   }
   if (global_reference.empty()) {
      throw std::runtime_error("No genome in " + reference_genome_path.string());
   }
   return global_reference;
};

std::unordered_map<std::string, std::string> initAliasKey(const std::string& working_directory) {
   std::filesystem::path const alias_key_path(working_directory + PANGO_ALIAS_FILENAME);
   if (!std::filesystem::exists(alias_key_path)) {
      throw std::filesystem::filesystem_error(
         "Alias key file " + alias_key_path.relative_path().string() + " does not exist",
         std::error_code()
      );
   }

   std::unordered_map<std::string, std::string> alias_keys;
   std::ifstream alias_key_file(alias_key_path.relative_path());
   while (true) {
      std::string alias;
      std::string val;
      if (!getline(alias_key_file, alias, '\t')) {
         break;
      }
      if (!getline(alias_key_file, val, '\n')) {
         break;
      }
      alias_keys[alias] = val;
   }

   return alias_keys;
}

silo::Database::Database(const std::string& directory)
    : working_directory(directory),
      global_reference(initGlobalReference(directory)),
      alias_key(initAliasKey(directory)) {}

const std::unordered_map<std::string, std::string>& silo::Database::getAliasKey() const {
   return alias_key;
}

void silo::Database::build(
   const std::string& partition_name_prefix,
   const std::string& metadata_file_suffix,
   const std::string& sequence_file_suffix,
   std::ostream& out
) {
   int64_t micros = 0;
   {
      BlockTimer const timer(micros);
      partitions.resize(partition_descriptor->partitions.size());
      tbb::parallel_for(
         static_cast<size_t>(0), partition_descriptor->partitions.size(),
         [&](size_t partition_index) {
            const auto& part = partition_descriptor->partitions[partition_index];
            partitions[partition_index].chunks = part.chunks;
            for (unsigned chunk_index = 0; chunk_index < part.chunks.size(); ++chunk_index) {
               std::string name;
               name = partition_name_prefix + buildChunkName(partition_index, chunk_index);
               std::string sequence_filename = name + sequence_file_suffix;
               std::ifstream meta_in(name + metadata_file_suffix);
               if (!InputStreamWrapper(sequence_filename).getInputStream()) {
                  sequence_filename += ".xz";
                  if (!InputStreamWrapper(sequence_filename).getInputStream()) {
                     std::osyncstream(std::cerr)
                        << "Sequence_file " << (name + sequence_file_suffix) << " not found"
                        << std::endl;
                     return;
                  }
                  std::osyncstream(std::cerr)
                     << "Using sequence_file " << (sequence_filename) << std::endl;
               } else {
                  std::osyncstream(std::cerr)
                     << "Using sequence_file " << (sequence_filename) << std::endl;
               }
               if (!meta_in) {
                  std::osyncstream(std::cerr) << "Meta_in file " << (name + metadata_file_suffix)
                                              << " not found" << std::endl;
                  return;
               }
               silo::InputStreamWrapper const sequence_input(sequence_filename);
               std::osyncstream(std::cerr)
                  << "Using meta_in file " << (name + metadata_file_suffix) << std::endl;
               unsigned const count1 = fillSequenceStore(
                  partitions[partition_index].seq_store, sequence_input.getInputStream()
               );
               unsigned const count2 = fillMetadataStore(
                  partitions[partition_index].meta_store, meta_in, alias_key, *dict
               );
               if (count1 != count2) {
                  // Fatal error
                  std::osyncstream(std::cerr)
                     << "Sequences in meta data and sequence data for chunk "
                     << buildChunkName(partition_index, chunk_index) << " are not equal."
                     << std::endl;
                  std::osyncstream(std::cerr) << "Abort build." << std::endl;
                  throw std::runtime_error("Error");
               }
               partitions[partition_index].sequenceCount += count1;
            }
         }
      );
   }
   out << "Build took " << std::to_string(micros) << "seconds." << std::endl;
   out << "Info directly after build: " << std::endl;
   const auto info = getDatabaseInfo();
   out << "Sequence count: " << info.sequenceCount << std::endl;
   out << "Total size: " << info.totalSize << std::endl;
   out << "nucleotide_symbol_n_bitmaps per sequence, total size: "
       << formatNumber(info.nBitmapsSize) << std::endl;
   detailedDatabaseInfo(out);
   {
      BlockTimer const timer(micros);
      // Precompute Bitmaps for metadata.
      finalizeBuild();
   }
   out << "Index precomputation for metadata took " << std::to_string(micros) << "seconds."
       << std::endl;
}

void silo::DatabasePartition::finalizeBuild(const Dictionary& dict) {
   {  /// Precompute all bitmaps for pango_lineages and -sublineages
      const uint32_t pango_count = dict.getPangoLineageCount();
      std::vector<std::vector<uint32_t>> group_by_lineages(pango_count);
      for (uint32_t sid = 0; sid < sequenceCount; ++sid) {
         const auto lineage = meta_store.sequence_id_to_lineage.at(sid);
         group_by_lineages.at(lineage).push_back(sid);
      }

      meta_store.lineage_bitmaps.resize(pango_count);
      for (uint32_t pango = 0; pango < pango_count; ++pango) {
         meta_store.lineage_bitmaps[pango].addMany(
            group_by_lineages[pango].size(), group_by_lineages[pango].data()
         );
      }

      meta_store.sublineage_bitmaps.resize(pango_count);
      for (uint32_t pango1 = 0; pango1 < pango_count; ++pango1) {
         // Initialize with all lineages that are in pango1
         std::vector<uint32_t> group_by_lineages_sub(group_by_lineages[pango1]);

         // Now add all lineages that I am a prefix of
         for (uint32_t pango2 = 0; pango2 < pango_count; ++pango2) {
            const std::string& str1 = dict.getPangoLineage(pango1);
            const std::string& str2 = dict.getPangoLineage(pango2);
            if (str1.length() >= str2.length()) {
               continue;
            }
            // Check if str1 is a prefix of str2 -> str2 is a sublineage of str1
            if (str2.starts_with(str1)) {
               for (uint32_t const pid : group_by_lineages[pango2]) {
                  group_by_lineages_sub.push_back(pid);
               }
            }
         }
         // Sort, for roaring insert
         std::sort(group_by_lineages_sub.begin(), group_by_lineages_sub.end());
         meta_store.sublineage_bitmaps[pango1].addMany(
            group_by_lineages_sub.size(), group_by_lineages_sub.data()
         );
      }
   }

   {  /// Precompute all bitmaps for countries
      const uint32_t country_count = dict.getCountryCount();
      std::vector<std::vector<uint32_t>> group_by_country(country_count);
      for (uint32_t sid = 0; sid < sequenceCount; ++sid) {
         const auto& country = meta_store.sequence_id_to_country[sid];
         group_by_country[country].push_back(sid);
      }

      meta_store.country_bitmaps.resize(country_count);
      for (uint32_t country = 0; country < country_count; ++country) {
         meta_store.country_bitmaps[country].addMany(
            group_by_country[country].size(), group_by_country[country].data()
         );
      }
   }

   {  /// Precompute all bitmaps for regions
      const uint32_t region_count = dict.getRegionCount();
      std::vector<std::vector<uint32_t>> group_by_region(region_count);
      for (uint32_t sid = 0; sid < sequenceCount; ++sid) {
         const auto& region = meta_store.sequence_id_to_region[sid];
         group_by_region[region].push_back(sid);
      }

      meta_store.region_bitmaps.resize(region_count);
      for (uint32_t region = 0; region < region_count; ++region) {
         meta_store.region_bitmaps[region].addMany(
            group_by_region[region].size(), group_by_region[region].data()
         );
      }
   }
}
const std::vector<silo::Chunk>& silo::DatabasePartition::getChunks() const {
   return chunks;
}

void silo::Database::finalizeBuild() {
   tbb::parallel_for_each(partitions.begin(), partitions.end(), [&](DatabasePartition& partition) {
      partition.finalizeBuild(*dict);
   });
}

[[maybe_unused]] void silo::Database::flipBitmaps() {
   tbb::parallel_for_each(partitions.begin(), partitions.end(), [&](DatabasePartition& database_partition) {
      tbb::parallel_for(static_cast<unsigned>(0), GENOME_LENGTH, [&](unsigned partition_index) {
         unsigned max_symbol = UINT32_MAX;
         unsigned max_count = 0;

         for (unsigned symbol = 0; symbol <= GENOME_SYMBOL::N; ++symbol) {
            unsigned const count =
               database_partition.seq_store.positions[partition_index].bitmaps[symbol].cardinality(
               );
            if (count > max_count) {
               max_symbol = symbol;
               max_count = count;
            }
         }
         if (max_symbol == GENOME_SYMBOL::A || max_symbol == GENOME_SYMBOL::C || max_symbol == GENOME_SYMBOL::G || max_symbol == GENOME_SYMBOL::T || max_symbol == GENOME_SYMBOL::N) {
            database_partition.seq_store.positions[partition_index].flipped_bitmap = max_symbol;
            database_partition.seq_store.positions[partition_index].bitmaps[max_symbol].flip(
               0, database_partition.sequenceCount
            );
         }
      });
   });
}

using r_stat = roaring::api::roaring_statistics_t;

static inline void addStatistic(r_stat& statistic1, const r_stat& statistic_to_add) {
   statistic1.cardinality += statistic_to_add.cardinality;
   if (statistic_to_add.max_value > statistic1.max_value) {
      statistic1.max_value = statistic_to_add.max_value;
   }
   if (statistic_to_add.min_value < statistic1.min_value) {
      statistic1.min_value = statistic_to_add.min_value;
   }
   statistic1.n_array_containers += statistic_to_add.n_array_containers;
   statistic1.n_run_containers += statistic_to_add.n_run_containers;
   statistic1.n_bitset_containers += statistic_to_add.n_bitset_containers;
   statistic1.n_bytes_array_containers += statistic_to_add.n_bytes_array_containers;
   statistic1.n_bytes_run_containers += statistic_to_add.n_bytes_run_containers;
   statistic1.n_bytes_bitset_containers += statistic_to_add.n_bytes_bitset_containers;
   statistic1.n_values_array_containers += statistic_to_add.n_values_array_containers;
   statistic1.n_values_run_containers += statistic_to_add.n_values_run_containers;
   statistic1.n_values_bitset_containers += statistic_to_add.n_values_bitset_containers;
   statistic1.n_containers += statistic_to_add.n_containers;
   statistic1.sum_value += statistic_to_add.sum_value;
}

silo::DatabaseInfo silo::Database::getDatabaseInfo() {
   std::atomic<uint32_t> sequence_count = 0;
   std::atomic<uint64_t> total_size = 0;
   std::atomic<size_t> nucleotide_symbol_n_bitmaps_size = 0;

   tbb::parallel_for_each(
      partitions.begin(), partitions.end(),
      [&](const DatabasePartition& database_partition) {
         sequence_count += database_partition.sequenceCount;
         total_size += database_partition.seq_store.computeSize();
         for (const auto& bitmap : database_partition.seq_store.nucleotide_symbol_n_bitmaps) {
            nucleotide_symbol_n_bitmaps_size += bitmap.getSizeInBytes(false);
         }
      }
   );

   return silo::DatabaseInfo{sequence_count, total_size, nucleotide_symbol_n_bitmaps_size};
}

[[maybe_unused]] void silo::Database::indexAllNucleotideSymbolsN() {
   int64_t microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      tbb::parallel_for_each(
         partitions.begin(), partitions.end(),
         [&](DatabasePartition& database_partition) {
            database_partition.seq_store.indexAllNucleotideSymbolsN();
         }
      );
   }
   std::cerr << "index all N took " << formatNumber(microseconds) << " microseconds." << std::endl;
}

[[maybe_unused]] void silo::Database::naiveIndexAllNucleotideSymbolsN() {
   int64_t microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      tbb::parallel_for_each(partitions.begin(), partitions.end(), [&](DatabasePartition& dbp) {
         dbp.seq_store.naiveIndexAllNucleotideSymbolN();
      });
   }
   std::cerr << "index all N naive took " << formatNumber(microseconds) << " microseconds."
             << std::endl;
}

[[maybe_unused]] void silo::Database::printFlippedGenomePositions(std::ostream& output_file) {
   output_file << "Flipped genome positions: " << std::endl;
   for (unsigned part_id = 0; part_id < partitions.size(); ++part_id) {
      const DatabasePartition& dbp = partitions[part_id];
      for (unsigned i = 0; i < GENOME_LENGTH; ++i) {
         const Position& pos = dbp.seq_store.positions[i];
         if (pos.flipped_bitmap != silo::toNucleotideSymbol(global_reference[0].at(i))) {
            output_file << std::to_string(part_id) << ": " << std::to_string(i)
                        << SYMBOL_REPRESENTATION[pos.flipped_bitmap] << std::endl;
         }
      }
      output_file << std::endl;
   }
}

int silo::Database::detailedDatabaseInfo(std::ostream& output_file) {
   std::string csv_line_storage;
   std::string csv_line_containers;
   std::string csv_header_histogram;
   std::string csv_line_histogram;

   std::vector<size_t> size_by_symbols(SYMBOL_COUNT);

   tbb::parallel_for(static_cast<unsigned>(0), SYMBOL_COUNT, [&](unsigned symbol) {
      for (const DatabasePartition& dbp : partitions) {
         for (const auto& position : dbp.seq_store.positions) {
            size_by_symbols[symbol] += position.bitmaps[symbol].getSizeInBytes();
         }
      }
   });
   uint64_t size_sum = 0;
   for (unsigned symbol = 0; symbol < SYMBOL_COUNT; ++symbol) {
      size_sum += size_by_symbols[symbol];
      output_file << "size for symbol '" << SYMBOL_REPRESENTATION[symbol]
                  << "': " << formatNumber(size_by_symbols[symbol]) << std::endl;
      csv_line_storage += std::to_string(size_by_symbols[symbol]);
      csv_line_storage += ",";
   }
   csv_line_storage += std::to_string(size_sum) + ",";
   csv_line_storage += std::to_string(size_sum - size_by_symbols[GENOME_SYMBOL::N]) + ",";

   std::mutex lock;
   constexpr int GENOME_DIVISION = 500;
   std::vector<uint32_t> bitset_containers_by_500pos((GENOME_LENGTH / GENOME_DIVISION) + 1);
   std::vector<uint32_t> gap_bitset_containers_by_500pos((GENOME_LENGTH / GENOME_DIVISION) + 1);
   std::vector<uint32_t> nucleotide_symbol_n_bitset_containers_by_500pos(
      (GENOME_LENGTH / GENOME_DIVISION) + 1
   );
   r_stat total_statistic{};
   uint64_t total_size_comp = 0;
   uint64_t total_size_frozen = 0;
   /// Because the counters in r_stat are 32 bit and overflow..
   uint64_t n_bytes_array_containers = 0;
   uint64_t n_bytes_run_containers = 0;
   uint64_t n_bytes_bitset_containers = 0;

   tbb::parallel_for(static_cast<unsigned>(0), GENOME_LENGTH, [&](unsigned position_index) {
      r_stat s_local{};
      uint64_t total_size_comp_local = 0;
      uint64_t total_size_frozen_local = 0;
      uint64_t n_bytes_array_containers_local = 0;
      uint64_t n_bytes_run_containers_local = 0;
      uint64_t n_bytes_bitset_containers_local = 0;
      {
         r_stat statistic;
         for (const auto& partition : partitions) {
            const Position& position = partition.seq_store.positions[position_index];
            for (unsigned symbol_index = 0; symbol_index < SYMBOL_COUNT; ++symbol_index) {
               const roaring::Roaring& bitmap = position.bitmaps[symbol_index];
               roaring_bitmap_statistics(&bitmap.roaring, &statistic);
               addStatistic(s_local, statistic);
               total_size_comp_local += bitmap.getSizeInBytes();
               total_size_frozen_local += bitmap.getFrozenSizeInBytes();
               n_bytes_array_containers_local += statistic.n_bytes_array_containers;
               n_bytes_run_containers_local += statistic.n_bytes_run_containers;
               n_bytes_bitset_containers_local += statistic.n_bytes_bitset_containers;
               if (statistic.n_bitset_containers > 0) {
                  if (symbol_index == GENOME_SYMBOL::N) {
                     nucleotide_symbol_n_bitset_containers_by_500pos
                        [position_index / GENOME_DIVISION] += statistic.n_bitset_containers;
                  } else if (symbol_index == GENOME_SYMBOL::GAP) {
                     gap_bitset_containers_by_500pos[position_index / GENOME_DIVISION] +=
                        statistic.n_bitset_containers;
                  } else {
                     bitset_containers_by_500pos[position_index / GENOME_DIVISION] +=
                        statistic.n_bitset_containers;
                  }
               }
            }
         }
      }
      lock.lock();
      addStatistic(total_statistic, s_local);
      total_size_comp += total_size_comp_local;
      total_size_frozen += total_size_frozen_local;
      n_bytes_array_containers += n_bytes_array_containers_local;
      n_bytes_run_containers += n_bytes_run_containers_local;
      n_bytes_bitset_containers += n_bytes_bitset_containers_local;
      lock.unlock();
   });
   output_file << "Total bitmap containers " << formatNumber(total_statistic.n_containers)
               << ", of those there are " << std::endl
               << "array: " << formatNumber(total_statistic.n_array_containers) << std::endl
               << "run: " << formatNumber(total_statistic.n_run_containers) << std::endl
               << "bitset: " << formatNumber(total_statistic.n_bitset_containers) << std::endl;
   csv_line_containers += std::to_string(total_statistic.n_containers) + "," +
                          std::to_string(total_statistic.n_array_containers) + ",";
   csv_line_containers += std::to_string(total_statistic.n_run_containers) + "," +
                          std::to_string(total_statistic.n_bitset_containers) + ",";
   output_file << "Total bitmap values " << formatNumber(total_statistic.cardinality)
               << ", of those there are " << std::endl
               << "array: " << formatNumber(total_statistic.n_values_array_containers) << std::endl
               << "run: " << formatNumber(total_statistic.n_values_run_containers) << std::endl
               << "bitset: " << formatNumber(total_statistic.n_values_bitset_containers)
               << std::endl;
   csv_line_containers += std::to_string(total_statistic.cardinality) + "," +
                          std::to_string(total_statistic.n_values_array_containers) + ",";
   csv_line_containers += std::to_string(total_statistic.n_values_run_containers) + "," +
                          std::to_string(total_statistic.n_values_bitset_containers) + ",";

   uint64_t const total_size =
      n_bytes_array_containers + n_bytes_run_containers + n_bytes_bitset_containers;
   output_file << "Total bitmap byte size " << formatNumber(total_size_frozen) << " (frozen) "
               << std::endl;
   output_file << "Total bitmap byte size " << formatNumber(total_size_comp) << " (compute_size) "
               << std::endl;
   output_file << "Total bitmap byte size " << formatNumber(total_size) << ", of those there are "
               << std::endl
               << "array: " << formatNumber(total_statistic.n_bytes_array_containers) << std::endl
               << "run: " << formatNumber(total_statistic.n_bytes_run_containers) << std::endl
               << "bitset: " << formatNumber(total_statistic.n_bytes_bitset_containers)
               << std::endl;
   csv_line_containers += std::to_string(total_size) + "," +
                          std::to_string(total_statistic.n_bytes_array_containers) + ",";
   csv_line_containers += std::to_string(total_statistic.n_bytes_run_containers) + "," +
                          std::to_string(total_statistic.n_bytes_bitset_containers) + ",";

   output_file << "Bitmap distribution by position #NON_GAP (#GAP)" << std::endl;
   for (unsigned i = 0; i < (GENOME_LENGTH / GENOME_DIVISION) + 1; ++i) {
      uint32_t const gap_bitsets_at_pos = gap_bitset_containers_by_500pos[i];
      uint32_t const nucleotide_symbol_n_bitmaps_at_pos =
         nucleotide_symbol_n_bitset_containers_by_500pos[i];
      uint32_t const bitmaps_at_pos = bitset_containers_by_500pos[i];
      output_file << "Pos: [" << i * GENOME_DIVISION << "," << ((i + 1) * GENOME_DIVISION)
                  << "): " << bitmaps_at_pos << " (N: " << nucleotide_symbol_n_bitmaps_at_pos
                  << ", -: " << gap_bitsets_at_pos << ")" << '\n';
      csv_header_histogram += std::to_string(i * GENOME_DIVISION) + "-" +
                              std::to_string((i + 1) * GENOME_DIVISION) + ",";
      csv_header_histogram += std::to_string(i * GENOME_DIVISION) + "-" +
                              std::to_string((i + 1) * GENOME_DIVISION) + "N,";
      csv_header_histogram += std::to_string(i * GENOME_DIVISION) + "-" +
                              std::to_string((i + 1) * GENOME_DIVISION) + "-,";
      csv_line_histogram += std::to_string(bitmaps_at_pos) + "," +
                            std::to_string(nucleotide_symbol_n_bitmaps_at_pos) + "," +
                            std::to_string(gap_bitsets_at_pos) + ",";
   }

   output_file << "Storage:" << std::endl;
   output_file << csv_line_storage << std::endl;
   output_file << "Containers:" << std::endl;
   output_file << csv_line_containers << std::endl;
   output_file << csv_header_histogram << std::endl;
   output_file << csv_line_histogram << std::endl;
   return 0;
}

unsigned silo::fillSequenceStore(silo::SequenceStore& seq_store, std::istream& input_file) {
   static constexpr unsigned BUFFER_SIZE = 1024;

   unsigned sequence_count = 0;

   std::vector<std::string> genome_buffer;
   while (true) {
      std::string epi_isl;
      std::string genome;
      if (!getline(input_file, epi_isl)) {
         break;
      }
      if (!getline(input_file, genome)) {
         break;
      }
      if (genome.length() != GENOME_LENGTH) {
         std::cerr << "length mismatch!" << std::endl;
         throw std::runtime_error("length mismatch.");
      }

      genome_buffer.push_back(std::move(genome));
      if (genome_buffer.size() >= BUFFER_SIZE) {
         seq_store.interpret(genome_buffer);
         genome_buffer.clear();
      }

      ++sequence_count;
   }
   seq_store.interpret(genome_buffer);
   seq_store.databaseInfo(std::cout);

   return sequence_count;
}

unsigned silo::fillMetadataStore(
   MetadataStore& meta_store,
   std::istream& input_file,
   const std::unordered_map<std::string, std::string>& alias_key,
   const Dictionary& dict
) {
   // Ignore header line.
   input_file.ignore(LONG_MAX, '\n');

   unsigned sequence_count = 0;

   while (true) {
      std::string epi_isl;
      std::string pango_lineage_raw;
      std::string date;
      std::string region;
      std::string country;
      std::string division;
      if (!getline(input_file, epi_isl, '\t')) {
         break;
      }
      if (!getline(input_file, pango_lineage_raw, '\t')) {
         break;
      }
      if (!getline(input_file, date, '\t')) {
         break;
      }
      if (!getline(input_file, region, '\t')) {
         break;
      }
      if (!getline(input_file, country, '\t')) {
         break;
      }
      if (!getline(input_file, division, '\n')) {
         break;
      }

      /// Deal with pango_lineage alias:
      std::string const pango_lineage = resolvePangoLineageAlias(alias_key, pango_lineage_raw);

      constexpr int START_POSITION_OF_NUMBER_IN_EPI_ISL = 8;
      std::string const tmp = epi_isl.substr(START_POSITION_OF_NUMBER_IN_EPI_ISL);
      uint64_t const epi = stoi(tmp);

      struct std::tm time_struct {};
      std::istringstream time_stream(date);
      time_stream >> std::get_time(&time_struct, "%Y-%m-%d");
      std::time_t const time = mktime(&time_struct);

      std::vector<uint64_t> extra_cols;
      extra_cols.push_back(dict.getIdInGeneralLookup(division));

      silo::inputSequenceMeta(
         meta_store, epi, time, dict.getPangoLineageIdInLookup(pango_lineage),
         dict.getRegionIdInLookup(region), dict.getCountryIdInLookup(country), extra_cols
      );
      ++sequence_count;
   }

   return sequence_count;
}

void silo::savePangoLineageCounts(
   const silo::PangoLineageCounts& pango_lineage_counts,
   std::ostream& output_file
) {
   for (const auto& pango_lineage_count : pango_lineage_counts.pango_lineage_counts) {
      output_file << pango_lineage_count.pango_lineage << '\t' << pango_lineage_count.count << '\n';
   }
   output_file.flush();
}

silo::PangoLineageCounts silo::loadPangoLineageCounts(std::istream& input_stream) {
   silo::PangoLineageCounts descriptor;
   std::string lineage;
   std::string count_str;
   uint32_t count;
   while (input_stream && !input_stream.eof()) {
      if (!getline(input_stream, lineage, '\t')) {
         break;
      }
      if (!getline(input_stream, count_str, '\n')) {
         break;
      }
      count = atoi(count_str.c_str());
      descriptor.pango_lineage_counts.emplace_back(silo::PangoLineageCount{lineage, count});
   }
   return descriptor;
}

void silo::savePartitions(const silo::Partitions& partitions, std::ostream& output_file) {
   for (const auto& partition : partitions.partitions) {
      output_file << "P\t" << partition.name << '\t' << partition.chunks.size() << '\t'
                  << partition.count << '\n';
      for (const auto& chunk : partition.chunks) {
         output_file << "C\t" << chunk.prefix << '\t' << chunk.pango_lineages.size() << '\t'
                     << chunk.count << '\t' << chunk.offset << '\n';
         for (const auto& pango_lineage : chunk.pango_lineages) {
            output_file << "L\t" << pango_lineage << '\n';
         }
      }
   }
}

[[maybe_unused]] void silo::Database::saveDatabaseState(const std::string& save_directory) {
   if (!partition_descriptor) {
      std::cerr << "Cannot save database without partition_descriptor." << std::endl;
      return;
   }

   if (pango_descriptor) {
      std::ofstream pango_def_file(save_directory + "pango_descriptor.txt");
      if (!pango_def_file) {
         std::cerr << "Cannot open pango_descriptor output file: "
                   << (save_directory + "pango_descriptor.txt") << std::endl;
         return;
      }
      std::cout << "Save pango lineage descriptor to output file "
                << (save_directory + "pango_descriptor.txt") << std::endl;
      savePangoLineageCounts(*pango_descriptor, pango_def_file);
   }
   {
      std::ofstream part_def_file(save_directory + "partition_descriptor.txt");
      if (!part_def_file) {
         std::cerr << "Cannot open partition_descriptor output file: "
                   << (save_directory + "partition_descriptor.txt") << std::endl;
         return;
      }
      std::cout << "Save partitioning descriptor to output file "
                << (save_directory + "partition_descriptor.txt") << std::endl;
      savePartitions(*partition_descriptor, part_def_file);
   }
   {
      std::ofstream dict_output(save_directory + "dict.txt");
      if (!dict_output) {
         std::cerr << "Could not open '" << (save_directory + "dict.txt") << "'." << std::endl;
         return;
      }
      std::cout << "Save dictionary to output file " << (save_directory + "dict.txt") << std::endl;

      dict->saveDictionary(dict_output);
   }

   std::vector<std::ofstream> file_vec;
   for (unsigned i = 0; i < partition_descriptor->partitions.size(); ++i) {
      file_vec.emplace_back(save_directory + 'P' + std::to_string(i) + ".silo");

      if (!file_vec.back()) {
         std::cerr << "Cannot open partition output file for saving: "
                   << (save_directory + 'P' + std::to_string(i) + ".silo") << std::endl;
         return;
      }
   }
   std::cout << "Save partitions " << partitions.size() << std::endl;

   tbb::parallel_for(
      static_cast<size_t>(0), partition_descriptor->partitions.size(),
      [&](size_t partition_index) {
         ::boost::archive::binary_oarchive output_archive(file_vec[partition_index]);
         output_archive << partitions[partition_index];
      }
   );
   std::cout << "Finished saving partitions" << std::endl;
}
[[maybe_unused]] void silo::Database::loadDatabaseState(const std::string& save_directory) {
   std::ifstream part_def_file(save_directory + "partition_descriptor.txt");
   if (!part_def_file) {
      std::cerr << "Cannot open partition_descriptor input file for loading: "
                << (save_directory + "partition_descriptor.txt") << std::endl;
      return;
   }
   std::cout << "Load partitioning_def from input file "
             << (save_directory + "partition_descriptor.txt") << std::endl;
   partition_descriptor = std::make_unique<Partitions>(loadPartitions(part_def_file));

   std::ifstream pango_def_file(save_directory + "pango_descriptor.txt");
   if (pango_def_file) {
      std::cout << "Load pango_descriptor from input file "
                << (save_directory + "pango_descriptor.txt") << std::endl;
      pango_descriptor =
         std::make_unique<PangoLineageCounts>(loadPangoLineageCounts(pango_def_file));
   }

   {
      auto dict_input = std::ifstream(save_directory + "dict.txt");
      if (!dict_input) {
         std::cerr << "dict_input file " << (save_directory + "dict.txt") << " not found."
                   << std::endl;
         return;
      }
      std::cout << "Load dictionary from input file " << (save_directory + "dict.txt") << std::endl;
      dict = std::make_unique<Dictionary>(Dictionary::loadDictionary(dict_input));
   }

   std::cout << "Loading partitions from " << save_directory << std::endl;
   std::vector<std::ifstream> file_vec;
   for (unsigned i = 0; i < partition_descriptor->partitions.size(); ++i) {
      file_vec.emplace_back(save_directory + 'P' + std::to_string(i) + ".silo");

      if (!file_vec.back()) {
         std::cerr << "Cannot open partition input file for loading: "
                   << (save_directory + 'P' + std::to_string(i) + ".silo") << std::endl;
         return;
      }
   }

   partitions.resize(partition_descriptor->partitions.size());
   tbb::parallel_for(
      static_cast<size_t>(0), partition_descriptor->partitions.size(),
      [&](size_t partition_index) {
         ::boost::archive::binary_iarchive input_archive(file_vec[partition_index]);
         input_archive >> partitions[partition_index];
      }
   );
}
void silo::Database::preprocessing(const PreprocessingConfig& config) {
   std::cout << "buildPangoLineageCounts" << std::endl;
   std::ifstream metadata_stream(config.metadata_file.relative_path());
   pango_descriptor =
      std::make_unique<PangoLineageCounts>(silo::buildPangoLineageCounts(alias_key, metadata_stream)
      );

   std::cout << "buildPartitions" << std::endl;
   partition_descriptor = std::make_unique<Partitions>(
      silo::buildPartitions(*pango_descriptor, Architecture::MAX_PARTITIONS)
   );

   std::cout << "partitionSequences" << std::endl;
   std::ifstream metadata_stream2(config.metadata_file.relative_path());
   InputStreamWrapper const sequence_stream(config.sequence_file.relative_path());
   partitionSequences(
      *partition_descriptor, metadata_stream2, sequence_stream.getInputStream(),
      config.partition_folder.relative_path(), alias_key, config.metadata_file.extension(),
      config.sequence_file.extension()
   );

   std::cout << "sortChunks" << std::endl;
   silo::sortChunks(
      *partition_descriptor, config.partition_folder.relative_path(),
      config.metadata_file.extension(), config.sequence_file.extension()
   );

   std::cout << "Dictionary" << std::endl;
   dict = std::make_unique<Dictionary>();
   for (size_t partition_index = 0; partition_index < partition_descriptor->partitions.size();
        ++partition_index) {
      const auto& partition = partition_descriptor->partitions.at(partition_index);
      for (unsigned chunk_index = 0; chunk_index < partition.chunks.size(); ++chunk_index) {
         std::string const name = config.partition_folder.relative_path().string() +
                                  buildChunkName(partition_index, chunk_index) +
                                  config.metadata_file.extension().string();
         std::ifstream meta_in(name);
         if (!meta_in) {
            throw PreprocessingException("Meta_data file " + name + " not found.");
         }
         dict->updateDictionary(meta_in, getAliasKey());
      }
   }

   std::cout << "Build" << std::endl;
   build(
      config.partition_folder.relative_path(), config.metadata_file.extension(),
      config.sequence_file.extension(), std::cout
   );
}
silo::Database::Database() = default;

struct ThousandSeparator : std::numpunct<char> {
   [[nodiscard]] char_type do_thousands_sep() const override { return '\''; }
   [[nodiscard]] string_type do_grouping() const override { return "\3"; }
};

std::string silo::formatNumber(uint64_t number) {
   std::ostringstream oss;
   auto thousands = std::make_unique<ThousandSeparator>();
   oss.imbue(std::locale(oss.getloc(), thousands.release()));
   oss << number;
   return oss.str();
}
std::string silo::resolvePangoLineageAlias(
   const std::unordered_map<std::string, std::string>& alias_key,
   const std::string& pango_lineage
) {
   std::string pango_lineage_prefix;
   std::stringstream pango_lineage_stream(pango_lineage);
   getline(pango_lineage_stream, pango_lineage_prefix, '.');
   if (alias_key.contains(pango_lineage_prefix)) {
      if (pango_lineage_stream.eof()) {
         return alias_key.at(pango_lineage_prefix);
      }
      const std::string suffix(
         (std::istream_iterator<char>(pango_lineage_stream)), std::istream_iterator<char>()
      );
      return alias_key.at(pango_lineage_prefix) + '.' + suffix;
   }
   return pango_lineage;
}
std::string silo::buildChunkName(unsigned int partition, unsigned int chunk) {
   return "P" + std::to_string(partition) + "_C" + std::to_string(chunk);
}
