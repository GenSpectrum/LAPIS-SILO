#include "silo/database.h"

#include <spdlog/spdlog.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <iostream>
#include <roaring/roaring.hh>
#include <string>
#include <unordered_map>
#include <vector>

#include "external/PerfEvent.hpp"
#include "silo/common/format_number.h"
#include "silo/common/genome_symbols.h"
#include "silo/common/input_stream_wrapper.h"
#include "silo/common/log.h"
#include "silo/database_info.h"
#include "silo/persistence/exception.h"
#include "silo/prepare_dataset.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/storage/metadata_store.h"
#include "silo/storage/sequence_store.h"

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
         throw silo::persistence::LoadDatabaseException("No N in reference genome allowed.");
      }
      global_reference.push_back(line);
   }
   if (global_reference.empty()) {
      throw silo::persistence::LoadDatabaseException(
         "No genome in " + reference_genome_path.string()
      );
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

template <>
struct [[maybe_unused]] fmt::formatter<silo::DatabaseInfo> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(silo::DatabaseInfo database_info, format_context& ctx)
      -> decltype(ctx.out()) {
      return format_to(
         ctx.out(), "sequence count: {}, total size: {}, N bitmaps size: {}",
         database_info.sequence_count, silo::formatNumber(database_info.total_size),
         silo::formatNumber(database_info.n_bitmaps_size)
      );
   }
};

void silo::Database::build(
   const std::string& partition_name_prefix,
   const std::string& metadata_file_suffix,
   const std::string& sequence_file_suffix
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
                     SPDLOG_ERROR("Sequence file {} not found", name + sequence_file_suffix);
                     return;
                  }
                  SPDLOG_DEBUG("Using sequence file: {}", sequence_filename);
               } else {
                  SPDLOG_DEBUG("Using sequence file: {}", sequence_filename);
               }
               if (!meta_in) {
                  SPDLOG_ERROR("metadata file {} not found", name + metadata_file_suffix);
                  return;
               }
               silo::InputStreamWrapper const sequence_input(sequence_filename);
               SPDLOG_DEBUG("Using metadata file: {}", name + metadata_file_suffix);
               unsigned const sequence_store_sequence_count = fillSequenceStore(
                  partitions[partition_index].seq_store, sequence_input.getInputStream()
               );
               unsigned const metadata_store_sequence_count = fillMetadataStore(
                  partitions[partition_index].meta_store, meta_in, alias_key, *dict
               );
               if (sequence_store_sequence_count != metadata_store_sequence_count) {
                  throw silo::PreprocessingException(
                     "Sequences in meta data and sequence data for chunk " +
                     buildChunkName(partition_index, chunk_index) +
                     " are not equal. The sequence store has " +
                     std::to_string(sequence_store_sequence_count) +
                     " rows, the metadata store has " +
                     std::to_string(metadata_store_sequence_count) + " rows."
                  );
               }
               partitions[partition_index].sequenceCount += sequence_store_sequence_count;
            }
         }
      );
   }

   SPDLOG_INFO("Build took {} ms", micros);
   SPDLOG_INFO("database info: {}", getDatabaseInfo());

   {
      BlockTimer const timer(micros);
      // Precompute Bitmaps for metadata.
      finalizeBuild();
   }

   SPDLOG_INFO("Index precomputation for metadata took {} ms", micros);
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
         GENOME_SYMBOL max_symbol = GENOME_SYMBOL::UNDEFINED;
         unsigned max_count = 0;

         for (const auto& symbol : GENOME_SYMBOLS) {
            unsigned const count = database_partition.seq_store.positions[partition_index]
                                      .bitmaps[static_cast<unsigned>(symbol)]
                                      .cardinality();
            if (count > max_count) {
               max_symbol = symbol;
               max_count = count;
            }
         }
         if (max_symbol == GENOME_SYMBOL::A || max_symbol == GENOME_SYMBOL::C || max_symbol == GENOME_SYMBOL::G || max_symbol == GENOME_SYMBOL::T || max_symbol == GENOME_SYMBOL::N) {
            database_partition.seq_store.positions[partition_index].flipped_bitmap_for_symbol =
               max_symbol;
            database_partition.seq_store.positions[partition_index]
               .bitmaps[static_cast<unsigned>(max_symbol)]
               .flip(0, database_partition.sequenceCount);
         }
      });
   });
}

using r_stat = roaring::api::roaring_statistics_t;

silo::DatabaseInfo silo::Database::getDatabaseInfo() const {
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
   LOG_PERFORMANCE("index all N took {} microseconds", silo::formatNumber(microseconds));
}

[[maybe_unused]] void silo::Database::naiveIndexAllNucleotideSymbolsN() {
   int64_t microseconds = 0;
   {
      BlockTimer const timer(microseconds);
      tbb::parallel_for_each(partitions.begin(), partitions.end(), [&](DatabasePartition& dbp) {
         dbp.seq_store.naiveIndexAllNucleotideSymbolN();
      });
   }
   LOG_PERFORMANCE("index all N naive took {} microseconds", silo::formatNumber(microseconds));
}

silo::BitmapContainerSize::BitmapContainerSize(uint32_t section_length)
    : section_length(section_length),
      bitmap_container_size_statistic({0, 0, 0, 0, 0, 0, 0, 0, 0}),
      total_bitmap_size_frozen(0),
      total_bitmap_size_computed(0) {
   size_per_genome_symbol_and_section["NOT_N_NOT_GAP"] =
      std::vector<uint32_t>((GENOME_LENGTH / section_length) + 1, 0);
   size_per_genome_symbol_and_section[genomeSymbolRepresentation(GENOME_SYMBOL::GAP)] =
      std::vector<uint32_t>((GENOME_LENGTH / section_length) + 1, 0);
   size_per_genome_symbol_and_section[genomeSymbolRepresentation(GENOME_SYMBOL::N)] =
      std::vector<uint32_t>((GENOME_LENGTH / section_length) + 1, 0);
}

silo::BitmapContainerSize& silo::BitmapContainerSize::operator+=(
   const silo::BitmapContainerSize& other
) {
   if (this->section_length != other.section_length) {
      throw std::runtime_error("Cannot add BitmapContainerSize with different section lengths.");
   }
   this->total_bitmap_size_frozen += other.total_bitmap_size_frozen;
   this->total_bitmap_size_computed += other.total_bitmap_size_computed;

   for (const auto& map_entry : this->size_per_genome_symbol_and_section) {
      const auto symbol = map_entry.first;
      for (unsigned i = 0; i < this->size_per_genome_symbol_and_section.at(symbol).size(); ++i) {
         this->size_per_genome_symbol_and_section.at(symbol).at(i) +=
            other.size_per_genome_symbol_and_section.at(symbol).at(i);
      }
   }

   this->bitmap_container_size_statistic.number_of_bitset_containers +=
      other.bitmap_container_size_statistic.number_of_bitset_containers;
   this->bitmap_container_size_statistic.number_of_array_containers +=
      other.bitmap_container_size_statistic.number_of_array_containers;
   this->bitmap_container_size_statistic.number_of_run_containers +=
      other.bitmap_container_size_statistic.number_of_run_containers;

   this->bitmap_container_size_statistic.number_of_values_stored_in_array_containers +=
      other.bitmap_container_size_statistic.number_of_values_stored_in_array_containers;
   this->bitmap_container_size_statistic.number_of_values_stored_in_run_containers +=
      other.bitmap_container_size_statistic.number_of_values_stored_in_run_containers;
   this->bitmap_container_size_statistic.number_of_values_stored_in_bitset_containers +=
      other.bitmap_container_size_statistic.number_of_values_stored_in_bitset_containers;

   this->bitmap_container_size_statistic.total_bitmap_size_array_containers +=
      other.bitmap_container_size_statistic.total_bitmap_size_array_containers;
   this->bitmap_container_size_statistic.total_bitmap_size_run_containers +=
      other.bitmap_container_size_statistic.total_bitmap_size_run_containers;
   this->bitmap_container_size_statistic.total_bitmap_size_bitset_containers +=
      other.bitmap_container_size_statistic.total_bitmap_size_bitset_containers;

   return *this;
}

silo::BitmapSizePerSymbol& silo::BitmapSizePerSymbol::operator+=(
   const silo::BitmapSizePerSymbol& other
) {
   for (const auto& symbol : GENOME_SYMBOLS) {
      this->size_in_bytes.at(symbol) += other.size_in_bytes.at(symbol);
   }
   return *this;
}
silo::BitmapSizePerSymbol::BitmapSizePerSymbol() {
   for (const auto& symbol : GENOME_SYMBOLS) {
      this->size_in_bytes[symbol] = 0;
   }
}

silo::BitmapSizePerSymbol silo::Database::calculateBitmapSizePerSymbol() const {
   BitmapSizePerSymbol global_bitmap_size_per_symbol;

   std::mutex lock;
   tbb::parallel_for_each(GENOME_SYMBOLS, [&](GENOME_SYMBOL symbol) {
      BitmapSizePerSymbol bitmap_size_per_symbol;

      for (const DatabasePartition& database_partition : partitions) {
         for (const auto& position : database_partition.seq_store.positions) {
            bitmap_size_per_symbol.size_in_bytes[symbol] +=
               position.bitmaps[static_cast<unsigned>(symbol)].getSizeInBytes();
         }
      }
      lock.lock();
      global_bitmap_size_per_symbol += bitmap_size_per_symbol;
      lock.unlock();
   });

   return global_bitmap_size_per_symbol;
}

void addStatisticToBitmapContainerSize(
   const r_stat& statistic,
   silo::BitmapContainerSizeStatistic& size_statistic
) {
   size_statistic.number_of_array_containers += statistic.n_array_containers;
   size_statistic.number_of_run_containers += statistic.n_run_containers;
   size_statistic.number_of_bitset_containers += statistic.n_bitset_containers;

   size_statistic.total_bitmap_size_array_containers += statistic.n_bytes_array_containers;
   size_statistic.total_bitmap_size_run_containers += statistic.n_bytes_run_containers;
   size_statistic.total_bitmap_size_bitset_containers += statistic.n_bytes_bitset_containers;

   size_statistic.number_of_values_stored_in_array_containers +=
      statistic.n_values_array_containers;
   size_statistic.number_of_values_stored_in_run_containers += statistic.n_values_run_containers;
   size_statistic.number_of_values_stored_in_bitset_containers +=
      statistic.n_values_bitset_containers;
}

silo::BitmapContainerSize silo::Database::calculateBitmapContainerSizePerGenomeSection(
   uint32_t section_length
) const {
   BitmapContainerSize global_bitmap_container_size_per_genome_section(section_length);

   std::mutex lock;
   tbb::parallel_for(static_cast<unsigned>(0), GENOME_LENGTH, [&](unsigned position_index) {
      BitmapContainerSize bitmap_container_size_per_genome_section(section_length);

      r_stat statistic;
      for (const auto& partition : partitions) {
         const auto& position = partition.seq_store.positions[position_index];
         for (const auto& genome_symbol : GENOME_SYMBOLS) {
            const auto& bitmap = position.bitmaps[static_cast<unsigned>(genome_symbol)];

            roaring_bitmap_statistics(&bitmap.roaring, &statistic);
            addStatisticToBitmapContainerSize(
               statistic, bitmap_container_size_per_genome_section.bitmap_container_size_statistic
            );

            bitmap_container_size_per_genome_section.total_bitmap_size_computed +=
               bitmap.getSizeInBytes();
            bitmap_container_size_per_genome_section.total_bitmap_size_frozen +=
               bitmap.getFrozenSizeInBytes();

            if (statistic.n_bitset_containers > 0) {
               if (genome_symbol == GENOME_SYMBOL::N) {
                  bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                     .at(genomeSymbolRepresentation(GENOME_SYMBOL::N))
                     .at(position_index / section_length) += statistic.n_bitset_containers;
               } else if (genome_symbol == GENOME_SYMBOL::GAP) {
                  bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                     .at(genomeSymbolRepresentation(GENOME_SYMBOL::GAP))
                     .at(position_index / section_length) += statistic.n_bitset_containers;
               } else {
                  bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                     .at("NOT_N_NOT_GAP")
                     .at(position_index / section_length) += statistic.n_bitset_containers;
               }
            }
         }
      }
      lock.lock();
      global_bitmap_container_size_per_genome_section += bitmap_container_size_per_genome_section;
      lock.unlock();
   });

   return global_bitmap_container_size_per_genome_section;
}

silo::DetailedDatabaseInfo silo::Database::detailedDatabaseInfo(uint32_t section_length) const {
   BitmapSizePerSymbol const bitmap_size_per_symbol = calculateBitmapSizePerSymbol();
   BitmapContainerSize const size_per_section =
      calculateBitmapContainerSizePerGenomeSection(section_length);

   return DetailedDatabaseInfo{bitmap_size_per_symbol, size_per_section};
}

unsigned silo::fillSequenceStore(silo::SequenceStore& sequence_store, std::istream& input_file) {
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
         throw silo::PreprocessingException(
            "Error filling sequence store: Genome length was " + std::to_string(genome.length()) +
            ", expected " + std::to_string(GENOME_LENGTH)
         );
      }

      genome_buffer.push_back(std::move(genome));
      if (genome_buffer.size() >= BUFFER_SIZE) {
         sequence_store.interpret(genome_buffer);
         genome_buffer.clear();
      }

      ++sequence_count;
   }
   sequence_store.interpret(genome_buffer);
   SPDLOG_DEBUG("{}", sequence_store.getInfo());

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
      throw silo::persistence::SaveDatabaseException(
         "Cannot save database without partition_descriptor."
      );
   }

   if (pango_descriptor) {
      std::ofstream pango_def_file(save_directory + "pango_descriptor.txt");
      if (!pango_def_file) {
         throw silo::persistence::SaveDatabaseException(
            "Cannot open pango_descriptor output file " + save_directory + "pango_descriptor.txt"
         );
      }
      SPDLOG_INFO("Saving pango lineage descriptor to {}pango_descriptor.txt", save_directory);
      savePangoLineageCounts(*pango_descriptor, pango_def_file);
   }
   {
      std::ofstream part_def_file(save_directory + "partition_descriptor.txt");
      if (!part_def_file) {
         throw silo::persistence::SaveDatabaseException(
            "Cannot open partitioning descriptor output file " + save_directory +
            "partition_descriptor.txt"
         );
      }
      SPDLOG_INFO("Saving partitioning descriptor to {}partition_descriptor.txt", save_directory);
      savePartitions(*partition_descriptor, part_def_file);
   }
   {
      std::ofstream dict_output(save_directory + "dict.txt");
      if (!dict_output) {
         throw silo::persistence::SaveDatabaseException(
            "Cannot open dictionary output file " + save_directory + "dict.txt"
         );
      }
      SPDLOG_INFO("Saving dictionary to {}dict.txt", save_directory);

      dict->saveDictionary(dict_output);
   }

   std::vector<std::ofstream> file_vec;
   for (unsigned i = 0; i < partition_descriptor->partitions.size(); ++i) {
      const auto& partition_file = save_directory + 'P' + std::to_string(i) + ".silo";
      file_vec.emplace_back(partition_file);

      if (!file_vec.back()) {
         throw silo::persistence::SaveDatabaseException(
            "Cannot open partition output file " + partition_file + " for saving"
         );
      }
   }

   SPDLOG_INFO("Saving {} partitions...", partitions.size());

   tbb::parallel_for(
      static_cast<size_t>(0), partition_descriptor->partitions.size(),
      [&](size_t partition_index) {
         ::boost::archive::binary_oarchive output_archive(file_vec[partition_index]);
         output_archive << partitions[partition_index];
      }
   );
   SPDLOG_INFO("Finished saving partitions", partitions.size());
}

[[maybe_unused]] void silo::Database::loadDatabaseState(const std::string& save_directory) {
   const auto partition_descriptor_file = save_directory + "partition_descriptor.txt";
   std::ifstream part_def_file(partition_descriptor_file);
   if (!part_def_file) {
      throw silo::persistence::LoadDatabaseException(
         "Cannot open partition_descriptor input file for loading: " + partition_descriptor_file
      );
   }
   SPDLOG_INFO("Loading partitioning definition from {}", partition_descriptor_file);

   partition_descriptor = std::make_unique<Partitions>(loadPartitions(part_def_file));

   const auto pango_definition_file = save_directory + "pango_descriptor.txt";
   std::ifstream pango_def_file(pango_definition_file);
   if (pango_def_file) {
      SPDLOG_INFO("Loading pango definition from {}", pango_definition_file);
      pango_descriptor =
         std::make_unique<PangoLineageCounts>(loadPangoLineageCounts(pango_def_file));
   }

   {
      const auto dictionary_file = save_directory + "dict.txt";
      auto dict_input = std::ifstream(dictionary_file);
      if (!dict_input) {
         throw silo::persistence::LoadDatabaseException(
            "Cannot open dictionary input file for loading: " + dictionary_file
         );
      }
      SPDLOG_INFO("Loading dictionary from {}", dictionary_file);
      dict = std::make_unique<Dictionary>(Dictionary::loadDictionary(dict_input));
   }

   SPDLOG_INFO("Loading partitions from {}", save_directory);
   std::vector<std::ifstream> file_vec;
   for (unsigned i = 0; i < partition_descriptor->partitions.size(); ++i) {
      const auto partition_file = save_directory + 'P' + std::to_string(i) + ".silo";
      file_vec.emplace_back(partition_file);

      if (!file_vec.back()) {
         throw silo::persistence::LoadDatabaseException(
            "Cannot open partition input file for loading: " + partition_file
         );
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
   SPDLOG_INFO("preprocessing - building pango lineage counts");
   std::ifstream metadata_stream(config.metadata_file.relative_path());
   pango_descriptor =
      std::make_unique<PangoLineageCounts>(silo::buildPangoLineageCounts(alias_key, metadata_stream)
      );

   SPDLOG_INFO("preprocessing - building partitions");
   partition_descriptor = std::make_unique<Partitions>(
      silo::buildPartitions(*pango_descriptor, Architecture::MAX_PARTITIONS)
   );

   SPDLOG_INFO("preprocessing - partitioning sequences");
   std::ifstream metadata_stream2(config.metadata_file.relative_path());
   InputStreamWrapper const sequence_stream(config.sequence_file.relative_path());
   partitionSequences(
      *partition_descriptor, metadata_stream2, sequence_stream.getInputStream(),
      config.partition_folder.relative_path(), alias_key, config.metadata_file.extension(),
      config.sequence_file.extension()
   );

   SPDLOG_INFO("preprocessing - sorting chunks");
   silo::sortChunks(
      *partition_descriptor, config.partition_folder.relative_path(),
      config.metadata_file.extension(), config.sequence_file.extension()
   );

   SPDLOG_INFO("preprocessing - building dictionary");
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

   SPDLOG_INFO("preprocessing - building database");
   build(
      config.partition_folder.relative_path(), config.metadata_file.extension(),
      config.sequence_file.extension()
   );
}
silo::Database::Database() = default;

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
