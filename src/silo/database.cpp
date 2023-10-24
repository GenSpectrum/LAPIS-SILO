#include "silo/database.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <fmt/core.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_for_each.h>
#include <spdlog/spdlog.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/detail/interface_iarchive.hpp>
#include <boost/archive/detail/interface_oarchive.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/level_enum.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/tracking_enum.hpp>
#include <boost/serialization/vector.hpp>
#include <duckdb.hpp>
#include <roaring/roaring.hh>

#include "silo/common/block_timer.h"
#include "silo/common/data_version.h"
#include "silo/common/fasta_reader.h"
#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/database_config.h"
#include "silo/database_info.h"
#include "silo/persistence/exception.h"
#include "silo/prepare_dataset.h"
#include "silo/preprocessing/metadata_validator.h"
#include "silo/preprocessing/ndjson_digestion.h"
#include "silo/preprocessing/pango_lineage_count.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/query_engine/query_engine.h"
#include "silo/query_engine/query_result.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/insertion_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"
#include "silo/storage/serialize_optional.h"
#include "silo/zstdfasta/zstd_decompressor.h"
#include "silo/zstdfasta/zstdfasta_table.h"
#include "silo/zstdfasta/zstdfasta_table_reader.h"

template <>
struct [[maybe_unused]] fmt::formatter<silo::DatabaseInfo> : fmt::formatter<std::string> {
   [[maybe_unused]] static auto format(silo::DatabaseInfo database_info, format_context& ctx)
      -> decltype(ctx.out()) {
      return format_to(
         ctx.out(),
         "sequence count: {}, total size: {}, N bitmaps size: {}",
         database_info.sequence_count,
         silo::formatNumber(database_info.total_size),
         silo::formatNumber(database_info.n_bitmaps_size)
      );
   }
};

namespace silo {

template <>
std::optional<std::string> Database::getDefaultSequenceName<Nucleotide>() const {
   return database_config.default_nucleotide_sequence;
}

template <>
std::optional<std::string> Database::getDefaultSequenceName<AminoAcid>() const {
   return std::nullopt;
}

template <>
std::vector<std::string> Database::getSequenceNames<Nucleotide>() const {
   std::vector<std::string> sequence_names;
   for (const auto& [name, _] : nuc_sequences) {
      sequence_names.emplace_back(name);
   }
   return sequence_names;
}

template <>
std::vector<std::string> Database::getSequenceNames<AminoAcid>() const {
   std::vector<std::string> sequence_names;
   for (const auto& [name, _] : aa_sequences) {
      sequence_names.emplace_back(name);
   }
   return sequence_names;
}

template <>
const std::map<std::string, SequenceStore<Nucleotide>>& Database::getSequenceStores<Nucleotide>(
) const {
   return nuc_sequences;
}

template <>
const std::map<std::string, SequenceStore<AminoAcid>>& Database::getSequenceStores<AminoAcid>(
) const {
   return aa_sequences;
}

const PangoLineageAliasLookup& Database::getAliasKey() const {
   return alias_key;
}

void Database::build(
   duckdb::Connection& connection,
   const preprocessing::Partitions& partition_descriptor,
   const ReferenceGenomes& reference_genomes,
   const std::string& order_by_clause
) {
   int64_t micros = 0;
   {
      const BlockTimer timer(micros);
      for (const auto& partition : partition_descriptor.getPartitions()) {
         partitions.emplace_back(partition.getPartitionChunks());
      }
      initializeColumns();
      initializeNucSequences(reference_genomes.nucleotide_sequences);
      initializeAASequences(reference_genomes.aa_sequences);

      SPDLOG_INFO("build - building metadata store");

      for (size_t partition_id = 0; partition_id < partition_descriptor.getPartitions().size();
           ++partition_id) {
         const auto& part = partition_descriptor.getPartitions()[partition_id];
         for (size_t chunk_index = 0; chunk_index < part.getPartitionChunks().size();
              ++chunk_index) {
            partitions[partition_id].sequence_count += partitions[partition_id].columns.fill(
               connection, partition_id, order_by_clause, database_config
            );
         }
         SPDLOG_INFO("build - finished columns for partition {}", partition_id);
      }

      SPDLOG_INFO("build - building sequence stores");

      tbb::parallel_for(
         tbb::blocked_range<size_t>(0, partition_descriptor.getPartitions().size()),
         [&](const auto& local) {
            for (auto partition_index = local.begin(); partition_index != local.end();
                 ++partition_index) {
               const auto& part = partition_descriptor.getPartitions()[partition_index];
               for (size_t chunk_index = 0; chunk_index < part.getPartitionChunks().size();
                    ++chunk_index) {
                  for (const auto& [nuc_name, reference_sequence] :
                       reference_genomes.raw_nucleotide_sequences) {
                     SPDLOG_DEBUG(
                        "build - building sequence store for nucleotide sequence {} and partition "
                        "{}",
                        nuc_name,
                        partition_index
                     );

                     silo::ZstdFastaTableReader sequence_input(
                        connection,
                        "nuc_" + nuc_name,
                        reference_sequence,
                        fmt::format("partition_id = {}", partition_index),
                        ""  // TODO order
                     );
                     partitions[partition_index].nuc_sequences.at(nuc_name).fill(sequence_input);
                  }
                  for (const auto& [aa_name, reference_sequence] :
                       reference_genomes.raw_aa_sequences) {
                     SPDLOG_DEBUG(
                        "build - building sequence store for amino acid sequence {} and partition "
                        "{}",
                        aa_name,
                        partition_index
                     );

                     silo::ZstdFastaTableReader sequence_input(
                        connection,
                        "gene_" + aa_name,
                        reference_sequence,
                        fmt::format("partition_id = {}", partition_index),
                        ""  // TODO order
                     );
                     partitions[partition_index].aa_sequences.at(aa_name).fill(sequence_input);
                  }
               }
               partitions.at(partition_index).flipBitmaps();
               SPDLOG_INFO("build - finished sequences for partition {}", partition_index);
            }
         }
      );
      finalizeInsertionIndexes();
   }

   SPDLOG_INFO("Build took {} ms", micros);
   SPDLOG_INFO("database info: {}", getDatabaseInfo());
}

using RoaringStatistics = roaring::api::roaring_statistics_t;

DatabaseInfo Database::getDatabaseInfo() const {
   std::atomic<uint32_t> sequence_count = 0;
   std::atomic<uint64_t> total_size = 0;
   std::atomic<size_t> nucleotide_symbol_n_bitmaps_size = 0;

   tbb::parallel_for_each(
      partitions.begin(),
      partitions.end(),
      [&](const DatabasePartition& database_partition) {
         uint64_t local_total_size = 0;
         size_t local_nucleotide_symbol_n_bitmaps_size = 0;
         for (const auto& [_, seq_store] : database_partition.nuc_sequences) {
            local_total_size += seq_store.computeSize();
            for (const auto& bitmap : seq_store.missing_symbol_bitmaps) {
               local_nucleotide_symbol_n_bitmaps_size += bitmap.getSizeInBytes(false);
            }
         }
         sequence_count += database_partition.sequence_count;
         total_size += local_total_size;
         nucleotide_symbol_n_bitmaps_size += local_nucleotide_symbol_n_bitmaps_size;
      }
   );

   return DatabaseInfo{sequence_count, total_size, nucleotide_symbol_n_bitmaps_size};
}

BitmapContainerSize::BitmapContainerSize(size_t genome_length, size_t section_length)
    : section_length(section_length),
      bitmap_container_size_statistic({0, 0, 0, 0, 0, 0, 0, 0, 0}),
      total_bitmap_size_frozen(0),
      total_bitmap_size_computed(0) {
   size_per_genome_symbol_and_section["NOT_N_NOT_GAP"] =
      std::vector<size_t>((genome_length / section_length) + 1, 0);
   size_per_genome_symbol_and_section["-"] =
      std::vector<size_t>((genome_length / section_length) + 1, 0);
   size_per_genome_symbol_and_section["N"] =
      std::vector<size_t>((genome_length / section_length) + 1, 0);
}

BitmapContainerSize& BitmapContainerSize::operator+=(const BitmapContainerSize& other) {
   if (this->section_length != other.section_length) {
      throw std::runtime_error("Cannot add BitmapContainerSize with different section lengths.");
   }
   this->total_bitmap_size_frozen += other.total_bitmap_size_frozen;
   this->total_bitmap_size_computed += other.total_bitmap_size_computed;

   for (const auto& map_entry : this->size_per_genome_symbol_and_section) {
      const auto symbol = map_entry.first;
      for (size_t i = 0; i < this->size_per_genome_symbol_and_section.at(symbol).size(); ++i) {
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

BitmapSizePerSymbol& BitmapSizePerSymbol::operator+=(const BitmapSizePerSymbol& other) {
   for (const auto& symbol : Nucleotide::SYMBOLS) {
      this->size_in_bytes.at(symbol) += other.size_in_bytes.at(symbol);
   }
   return *this;
}
BitmapSizePerSymbol::BitmapSizePerSymbol() {
   for (const auto& symbol : Nucleotide::SYMBOLS) {
      this->size_in_bytes[symbol] = 0;
   }
}

template <typename SymbolType>
BitmapSizePerSymbol Database::calculateBitmapSizePerSymbol(
   const SequenceStore<SymbolType>& seq_store
) {
   BitmapSizePerSymbol global_bitmap_size_per_symbol;

   std::mutex lock;
   tbb::parallel_for_each(Nucleotide::SYMBOLS, [&](Nucleotide::Symbol symbol) {
      BitmapSizePerSymbol bitmap_size_per_symbol;

      for (const SequenceStorePartition<SymbolType>& seq_store_partition : seq_store.partitions) {
         for (const auto& position : seq_store_partition.positions) {
            bitmap_size_per_symbol.size_in_bytes[symbol] +=
               position.bitmaps.at(symbol).getSizeInBytes();
         }
      }
      lock.lock();
      global_bitmap_size_per_symbol += bitmap_size_per_symbol;
      lock.unlock();
   });

   return global_bitmap_size_per_symbol;
}

void addStatisticToBitmapContainerSize(
   const RoaringStatistics& statistic,
   BitmapContainerSizeStatistic& size_statistic
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

template <typename SymbolType>
BitmapContainerSize Database::calculateBitmapContainerSizePerGenomeSection(
   const SequenceStore<SymbolType>& seq_store,
   size_t section_length
) {
   const uint32_t genome_length = seq_store.reference_sequence.size();

   BitmapContainerSize global_bitmap_container_size_per_genome_section(
      genome_length, section_length
   );

   std::mutex lock;
   tbb::parallel_for(tbb::blocked_range<uint32_t>(0U, genome_length), [&](const auto& range) {
      BitmapContainerSize bitmap_container_size_per_genome_section(genome_length, section_length);
      for (auto position_index = range.begin(); position_index != range.end(); ++position_index) {
         RoaringStatistics statistic;
         for (const auto& seq_store_partition : seq_store.partitions) {
            const auto& position = seq_store_partition.positions[position_index];
            for (const auto& genome_symbol : Nucleotide::SYMBOLS) {
               const auto& bitmap = position.bitmaps.at(genome_symbol);

               roaring_bitmap_statistics(&bitmap.roaring, &statistic);
               addStatisticToBitmapContainerSize(
                  statistic,
                  bitmap_container_size_per_genome_section.bitmap_container_size_statistic
               );

               bitmap_container_size_per_genome_section.total_bitmap_size_computed +=
                  bitmap.getSizeInBytes();
               bitmap_container_size_per_genome_section.total_bitmap_size_frozen +=
                  bitmap.getFrozenSizeInBytes();

               if (statistic.n_bitset_containers > 0) {
                  if (genome_symbol == Nucleotide::SYMBOL_MISSING) {
                     bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                        .at("N")
                        .at(position_index / section_length) += statistic.n_bitset_containers;
                  } else if (genome_symbol == Nucleotide::Symbol::GAP) {
                     bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                        .at("GAP")
                        .at(position_index / section_length) += statistic.n_bitset_containers;
                  } else {
                     bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                        .at("NOT_N_NOT_GAP")
                        .at(position_index / section_length) += statistic.n_bitset_containers;
                  }
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

DetailedDatabaseInfo Database::detailedDatabaseInfo() const {
   constexpr uint32_t DEFAULT_SECTION_LENGTH = 500;
   DetailedDatabaseInfo result;
   for (const auto& [seq_name, seq_store] : nuc_sequences) {
      result.sequences.insert(
         {seq_name,
          {BitmapSizePerSymbol{},
           BitmapContainerSize{seq_store.reference_sequence.size(), DEFAULT_SECTION_LENGTH}}}
      );
      result.sequences.at(seq_name).bitmap_size_per_symbol =
         calculateBitmapSizePerSymbol(seq_store);
      result.sequences.at(seq_name).bitmap_container_size_per_genome_section =
         calculateBitmapContainerSizePerGenomeSection(seq_store, DEFAULT_SECTION_LENGTH);
   }
   return result;
}

std::map<std::string, std::vector<Nucleotide::Symbol>> Database::getNucSequences() const {
   std::map<std::string, std::vector<Nucleotide::Symbol>> nucleotide_sequences_map;
   for (const auto& [name, store] : nuc_sequences) {
      nucleotide_sequences_map.emplace(name, store.reference_sequence);
   }
   return nucleotide_sequences_map;
}

std::map<std::string, std::vector<AminoAcid::Symbol>> Database::getAASequences() const {
   std::map<std::string, std::vector<AminoAcid::Symbol>> aa_sequences_map;
   for (const auto& [name, store] : aa_sequences) {
      aa_sequences_map.emplace(name, store.reference_sequence);
   }
   return aa_sequences_map;
}

namespace {

std::ifstream openInputFileOrThrow(const std::string& path) {
   std::ifstream file(path);
   if (!file) {
      auto error = fmt::format("Input file {} could not be opened.", path);
      throw persistence::LoadDatabaseException(error);
   }
   return file;
}

std::ofstream openOutputFileOrThrow(const std::string& path) {
   std::ofstream file(path);
   if (!file) {
      auto error = fmt::format("Output file {} could not be opened.", path);
      throw persistence::SaveDatabaseException(error);
   }
   return file;
}

}  // namespace

void saveDataVersion(const Database& database, const std::filesystem::path& save_directory) {
   std::ofstream data_version_file = openOutputFileOrThrow(save_directory / "data_version.silo");
   const auto data_version = database.getDataVersion().toString();
   data_version_file << data_version;
}

void Database::saveDatabaseState(const std::filesystem::path& save_directory) {
   if (getDataVersion().toString().empty()) {
      throw std::runtime_error(
         "Data version is empty. Please set a data version before saving the database."
      );
   }

   const std::filesystem::path versioned_save_directory =
      save_directory / getDataVersion().toString();

   if (std::filesystem::exists(versioned_save_directory)) {
      auto error = fmt::format(
         "In the output directory {} there already exists a file/folder with the name equal to "
         "the current data-version: {}",
         save_directory.string(),
         getDataVersion().toString()
      );
      throw persistence::LoadDatabaseException(error);
   }

   std::filesystem::create_directory(versioned_save_directory);

   const std::filesystem::path database_config_filename =
      versioned_save_directory / "database_config.yaml";
   database_config.writeConfig(database_config_filename);

   std::ofstream alias_key_file =
      openOutputFileOrThrow(versioned_save_directory / "alias_key.silo");
   ::boost::archive::binary_oarchive alias_key_archive(alias_key_file);
   alias_key_archive << alias_key;

   std::ofstream partitions_file =
      openOutputFileOrThrow(versioned_save_directory / "partitions.silo");
   ::boost::archive::binary_oarchive partitions_archive(partitions_file);
   partitions_archive << partitions;

   std::ofstream column_file = openOutputFileOrThrow(versioned_save_directory / "column_info.silo");
   ::boost::archive::binary_oarchive column_archive(column_file);
   column_archive << columns;

   auto nuc_sequences_map = getNucSequences();
   std::ofstream nuc_sequences_file =
      openOutputFileOrThrow(versioned_save_directory / "nuc_sequences.silo");
   ::boost::archive::binary_oarchive nuc_sequences_archive(nuc_sequences_file);
   nuc_sequences_archive << nuc_sequences_map;

   auto aa_sequences_map = getAASequences();
   std::ofstream aa_sequences_file =
      openOutputFileOrThrow(versioned_save_directory / "aa_sequences.silo");
   ::boost::archive::binary_oarchive aa_sequences_archive(aa_sequences_file);
   aa_sequences_archive << aa_sequences_map;

   std::vector<std::ofstream> file_vec;
   for (uint32_t i = 0; i < partitions.size(); ++i) {
      const auto& partition_file = versioned_save_directory / ("P" + std::to_string(i) + ".silo");
      file_vec.emplace_back(openOutputFileOrThrow(partition_file));

      if (!file_vec.back()) {
         throw persistence::SaveDatabaseException(
            "Cannot open partition output file " + partition_file.string() + " for saving"
         );
      }
   }

   SPDLOG_INFO("Saving {} partitions...", partitions.size());
   tbb::parallel_for(tbb::blocked_range<size_t>(0, partitions.size()), [&](const auto& local) {
      for (size_t partition_index = local.begin(); partition_index != local.end();
           partition_index++) {
         ::boost::archive::binary_oarchive output_archive(file_vec[partition_index]);
         partitions[partition_index].serializeData(output_archive, 0);
      }
   });
   SPDLOG_INFO("Finished saving partitions", partitions.size());

   saveDataVersion(*this, versioned_save_directory);
}

DataVersion loadDataVersion(const std::filesystem::path& filename) {
   std::ifstream data_version_file = openInputFileOrThrow(filename);
   std::string data_version_string;
   data_version_file >> data_version_string;
   auto data_version = DataVersion::fromString(data_version_string);
   if (data_version == std::nullopt) {
      auto error = fmt::format(
         "Data version file {} did not contain a valid data version: {}",
         filename.string(),
         data_version_string
      );
      SPDLOG_ERROR(error);
      throw persistence::LoadDatabaseException(error);
   }
   return data_version.value();
}

Database Database::loadDatabaseState(const std::filesystem::path& save_directory) {
   Database database;
   const auto database_config_filename = save_directory / "database_config.yaml";
   database.database_config =
      silo::config::DatabaseConfigReader().readConfig(database_config_filename);

   SPDLOG_TRACE("Loading alias key from {}", (save_directory / "alias_key.silo").string());
   std::ifstream alias_key_file = openInputFileOrThrow(save_directory / "alias_key.silo");
   ::boost::archive::binary_iarchive alias_key_archive(alias_key_file);
   alias_key_archive >> database.alias_key;

   SPDLOG_TRACE("Loading partitions from {}", (save_directory / "partitions.silo").string());
   std::ifstream partitions_file = openInputFileOrThrow(save_directory / "partitions.silo");
   ::boost::archive::binary_iarchive partitions_archive(partitions_file);
   partitions_archive >> database.partitions;

   SPDLOG_TRACE("Initializing columns");
   database.initializeColumns();

   SPDLOG_TRACE("Loading column info from {}", (save_directory / "column_info.silo").string());
   std::ifstream column_file = openInputFileOrThrow(save_directory / "column_info.silo");
   ::boost::archive::binary_iarchive column_archive(column_file);
   column_archive >> database.columns;

   SPDLOG_TRACE(
      "Loading nucleotide sequences from {}", (save_directory / "nuc_sequences.silo").string()
   );
   std::map<std::string, std::vector<Nucleotide::Symbol>> nuc_sequences_map;
   std::ifstream nuc_sequences_file = openInputFileOrThrow(save_directory / "nuc_sequences.silo");
   ::boost::archive::binary_iarchive nuc_sequences_archive(nuc_sequences_file);
   nuc_sequences_archive >> nuc_sequences_map;

   SPDLOG_TRACE(
      "Loading amino acid sequences from {}", (save_directory / "aa_sequences.silo").string()
   );
   std::map<std::string, std::vector<AminoAcid::Symbol>> aa_sequences_map;
   std::ifstream aa_sequences_file = openInputFileOrThrow(save_directory / "aa_sequences.silo");
   ::boost::archive::binary_iarchive aa_sequences_archive(aa_sequences_file);
   aa_sequences_archive >> aa_sequences_map;

   SPDLOG_INFO(
      "Finished loading partitions from {}", (save_directory / "aa_sequences.silo").string()
   );

   database.initializeNucSequences(nuc_sequences_map);
   database.initializeAASequences(aa_sequences_map);

   SPDLOG_DEBUG("Loading partition data");
   std::vector<std::ifstream> file_vec;
   for (uint32_t i = 0; i < database.partitions.size(); ++i) {
      const auto& partition_file = save_directory / ("P" + std::to_string(i) + ".silo");
      file_vec.emplace_back(openInputFileOrThrow(partition_file));

      if (!file_vec.back()) {
         throw persistence::SaveDatabaseException(
            "Cannot open partition input file " + partition_file.string() + " for loading"
         );
      }
   }

   tbb::parallel_for(
      tbb::blocked_range<size_t>(0, database.partitions.size()),
      [&](const auto& local) {
         for (size_t partition_index = local.begin(); partition_index != local.end();
              ++partition_index) {
            ::boost::archive::binary_iarchive input_archive(file_vec[partition_index]);
            database.partitions[partition_index].serializeData(input_archive, 0);
         }
      }
   );
   SPDLOG_INFO("Finished loading partition data");

   database.setDataVersion(loadDataVersion(save_directory / "data_version.silo"));
   SPDLOG_INFO(
      "Finished loading data_version from {}", (save_directory / "data_version.silo").string()
   );

   return database;
}

namespace {

preprocessing::Partitions buildPartitionDescriptorFromSQL(duckdb::Connection& connection) {
   auto partition_descriptor_from_sql =
      connection.Query("SELECT partition_id, count FROM partitioning ORDER BY partition_id");

   std::vector<preprocessing::Partition> partitions;

   uint32_t check_partition_id_sorted_and_contiguous = 0;

   for (auto it = partition_descriptor_from_sql->begin();
        it != partition_descriptor_from_sql->end();
        ++it) {
      const duckdb::Value db_partition_id = it.current_row.GetValue<duckdb::Value>(0);
      int64_t partition_id_int = duckdb::BigIntValue::Get(db_partition_id);
      if (partition_id_int != check_partition_id_sorted_and_contiguous) {
         throw silo::PreprocessingException(
            "The partition IDs produced by the preprocessing are not sorted, not starting from 0 "
            "or not contiguous."
         );
      }
      check_partition_id_sorted_and_contiguous++;
      uint32_t partition_id = partition_id_int;

      const duckdb::Value db_partition_size = it.current_row.GetValue<duckdb::Value>(1);
      int64_t partition_size_bigint = duckdb::BigIntValue::Get(db_partition_size);
      if (partition_size_bigint <= 0) {
         throw silo::PreprocessingException("Non-positive partition size encountered.");
      }
      if (partition_size_bigint > UINT32_MAX) {
         throw silo::PreprocessingException(
            fmt::format("Overflow of limit UINT32_MAX ({}) for number of sequences.", UINT32_MAX)
         );
      }

      uint32_t partition_size = static_cast<uint32_t>(partition_size_bigint);

      partitions.emplace_back(std::vector<preprocessing::PartitionChunk>{
         {partition_id, 0, partition_size, 0}});
   }

   return preprocessing::Partitions(partitions);
}

}  // namespace

Database Database::preprocessing(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const config::DatabaseConfig& database_config_
) {
   Database database;
   database.database_config = database_config_;

   const DataVersion& data_version = DataVersion::mineDataVersion();
   SPDLOG_INFO("preprocessing - mining data data_version: {}", data_version.toString());
   database.setDataVersion(data_version);

   SPDLOG_INFO("preprocessing - reading reference genome");
   const ReferenceGenomes& reference_genomes =
      ReferenceGenomes::readFromFile(preprocessing_config.getReferenceGenomeFilename());

   const std::optional<std::string> ndjson_input_filename =
      preprocessing_config.getNdjsonInputFilename();

   duckdb::DuckDB preprocessing_memory("test2.duckdb"
   );  // TODO make configurable via preprocessing config
   duckdb::Connection connection(preprocessing_memory);

   std::string order_by_clause;  // TODO turn into struct
   if (database_config_.schema.date_to_sort_by.has_value()) {
      SPDLOG_INFO("preprocessing - produce order by clause with a date to sort by");

      order_by_clause = fmt::format(
         "ORDER BY {}, {}",
         database_config_.schema.date_to_sort_by.value(),
         database_config_.schema.primary_key
      );
   } else {
      SPDLOG_INFO("preprocessing - produce order by clause without a date to sort by");

      order_by_clause = fmt::format("ORDER BY {}", database_config_.schema.primary_key);
   }
   SPDLOG_INFO("preprocessing - order by clause is {}", order_by_clause);

   if (ndjson_input_filename.has_value()) {
      SPDLOG_DEBUG("preprocessing - ndjson pipeline chosen");

      executeDuckDBRoutineForNdjsonDigestion(
         connection,
         preprocessing_config,
         reference_genomes,
         ndjson_input_filename.value(),
         database.database_config.schema.primary_key
      );

      auto return_code = connection.Query(
         "create or replace view metadata_table as\n"
         "select metadata.*, insertions, aaInsertions\n"
         "from preprocessing_table;"
      );
      if (return_code->HasError()) {
         SPDLOG_ERROR(return_code->GetError());
         throw silo::PreprocessingException(return_code->GetError());
      }

      SPDLOG_INFO("preprocessing - finished building the in-memory table for preprocessing");
      const std::string peek_query = "SELECT * FROM preprocessing_table LIMIT 5;";
      SPDLOG_TRACE(
         "preprocessing - peek into the table: {} \n {}",
         peek_query,
         connection.Query(peek_query)->ToString()
      );
   } else {
      SPDLOG_DEBUG("preprocessing - classic pipeline chosen");

      auto return_code = connection.Query(fmt::format(
         "create or replace table metadata_table as\n"
         "select *\n"
         "from '{}';",
         preprocessing_config.getMetadataInputFilename().string()
      ));
      if (return_code->HasError()) {
         SPDLOG_ERROR(return_code->GetError());
         throw silo::PreprocessingException(return_code->GetError());
      }
   }

   // TODO SPDLOG_INFO("preprocessing - validate metadata file (-> preprocessing_table) against
   // config");
   // TODO also validate types

   const std::string metadata_filename = preprocessing_config.getMetadataInputFilename().string();
   preprocessing::MetadataValidator().validateMedataFile(
      preprocessing_config.getMetadataInputFilename(), database_config_
   );

   SPDLOG_INFO("preprocessing - building alias key");
   const auto pango_lineage_definition_filename =
      preprocessing_config.getPangoLineageDefinitionFilename();
   if (pango_lineage_definition_filename.has_value()) {
      database.alias_key =
         PangoLineageAliasLookup::readFromFile(pango_lineage_definition_filename.value());
   }

   if (database_config_.schema.partition_by.has_value()) {
      SPDLOG_INFO("preprocessing - calculating partitions");

      auto return_code = connection.Query(fmt::format(
         "create\n"
         "or replace table partition_keys as\n"
         "select row_number() over () - 1 as id, *\n"
         "from (SELECT {} as partition_key, COUNT(*) as count "
         "      FROM metadata_table "
         "      GROUP BY partition_key "
         "      ORDER BY partition_key);",
         database_config_.schema.partition_by.value()  // TODO make sure partition key is validated
      ));

      if (return_code->HasError()) {
         throw silo::PreprocessingException(
            "Error in the execution of the duckdb statement for partition key table "
            "generation: " +
            return_code->GetError()
         );
      } else {
         SPDLOG_TRACE("Executed statement for partition key table generation.");
         SPDLOG_TRACE(return_code->ToString());
      }

      return_code = connection.Query(
         "create or replace table partitioning as\n"
         "with recursive "
         "          allowed_count(allowed_count) as (select sum(count) / 32 from "
         "partition_keys),\n"
         "          grouped_partition_keys(from_id, to_id, count) as\n"
         "              (select id, id, count\n"
         "               from partition_keys\n"
         "               where id = 0\n"
         "               union all\n"
         "               select case when l1.count <= allowed_count then l1.from_id else l2.id end,"
         "                      l2.id,\n"
         "                      case when l1.count <= allowed_count\n"
         "                           then l1.count + l2.count\n"
         "                           else l2.count end\n"
         "               from grouped_partition_keys l1,\n"
         "                    partition_keys l2,\n"
         "                    allowed_count\n"
         "               where l1.to_id + 1 = l2.id)\n"
         "select row_number() over () - 1 as partition_id, *\n"
         "from (select from_id, max(to_id) as to_id, max(count) as count\n"
         "      from grouped_partition_keys\n"
         "      group by from_id)"
      );
      if (return_code->HasError()) {
         SPDLOG_ERROR("Error when executing duckdb statement: {}", return_code->GetError());
         throw silo::PreprocessingException(return_code->GetError());
      }

      return_code = connection.Query(
         "create\n"
         "or replace view partition_key_to_partition as\n"
         "select partition_keys.partition_key as partition_key, "
         "  partitioning.partition_id as partition_id\n"
         "from partition_keys,\n"
         "     partitioning\n"
         "where partition_keys.id >= partitioning.from_id\n"
         "  AND partition_keys.id <= partitioning.to_id;"
      );
      if (return_code->HasError()) {
         SPDLOG_ERROR("Error when executing duckdb statement: {}", return_code->GetError());
         throw PreprocessingException(return_code->GetError());
      }

      return_code = connection.Query(fmt::format(
         "create\n"
         "or replace view partitioned_metadata as\n"
         "select partitioning.partition_id as partition_id, metadata_table.*\n"
         "from partition_keys,\n"
         "     partitioning,\n"
         "     metadata_table\n"
         "where (metadata_table.{0} = partition_keys.partition_key or (metadata_table.{0} is null "
         "and partition_keys.partition_key is null))\n"
         "  AND partition_keys.id >= partitioning.from_id\n"
         "  AND partition_keys.id <= partitioning.to_id;",
         database_config_.schema.partition_by.value()
      ));
      if (return_code->HasError()) {
         SPDLOG_ERROR("Error when executing duckdb statement: {}", return_code->GetError());
         throw PreprocessingException(return_code->GetError());
      }
   } else {
      SPDLOG_INFO(
         "preprocessing - skip partition merging because no partition_by key was provided, instead "
         "putting all sequences into the same partition"
      );

      auto return_code = connection.Query(
         "create or replace view partitioning as\n"
         "select 0 as partition_id, 0 as from_id, 0 as to_id, count(*) as count\n"
         "from metadata_table;"
      );

      if (return_code->HasError()) {
         SPDLOG_ERROR(return_code->GetError());
         throw silo::PreprocessingException(return_code->GetError());
      }

      return_code = connection.Query(
         "create\n"
         "or replace view partition_key_to_partition as\n"
         "as values (0, 0);"
      );

      if (return_code->HasError()) {
         SPDLOG_ERROR(return_code->GetError());
         throw silo::PreprocessingException(return_code->GetError());
      }

      return_code = connection.Query(
         "create\n"
         "or replace view partitioned_metadata as\n"
         "select 0 as partition_id, metadata_table.*\n"
         "from metadata_table;"
      );

      if (return_code->HasError()) {
         SPDLOG_ERROR(return_code->GetError());
         throw silo::PreprocessingException(return_code->GetError());
      }
   }

   if (preprocessing_config.getNdjsonInputFilename().has_value()) {
      for (const auto& [seq_name, _] : reference_genomes.raw_nucleotide_sequences) {
         auto return_code = connection.Query(fmt::format(
            "create or replace view nuc_{0} as\n"
            "select metadata.{1} as key, nuc_{0} as sequence,"
            "partition_key_to_partition.partition_id as partition_id\n"
            "from preprocessing_table, partition_key_to_partition "
            "where preprocessing_table.metadata.{2} = partition_key_to_partition.partition_key;",
            seq_name,
            database_config_.schema.primary_key,
            database_config_.schema.partition_by.value()
         ));

         if (return_code->HasError()) {
            SPDLOG_ERROR(return_code->GetError());
            throw silo::PreprocessingException(return_code->GetError());
         }
      }

      for (const auto& [seq_name, _] : reference_genomes.raw_aa_sequences) {
         auto return_code = connection.Query(fmt::format(
            "create or replace view gene_{0} as\n"
            "select metadata.{1} as key, gene_{0} as sequence, "
            "partition_key_to_partition.partition_id as partition_id\n"
            "from preprocessing_table, partition_key_to_partition "
            "where preprocessing_table.metadata.{2} = partition_key_to_partition.partition_key;",
            seq_name,
            database_config_.schema.primary_key,
            database_config_.schema.partition_by.value()
         ));

         if (return_code->HasError()) {
            SPDLOG_ERROR(return_code->GetError());
            throw silo::PreprocessingException(return_code->GetError());
         }
      }
   } else {
      for (const auto& [seq_name, reference_sequence] :
           reference_genomes.raw_nucleotide_sequences) {
         silo::FastaReader fasta_reader(preprocessing_config.getNucFilenameNoExtension(seq_name)
                                           .replace_extension(silo::preprocessing::FASTA_EXTENSION)
         );
         ZstdFastaTable::generate(
            connection, "raw_nuc_" + seq_name, fasta_reader, reference_sequence
         );

         auto return_code = connection.Query(fmt::format(
            "create or replace view nuc_{0} as\n"
            "select key, sequence,"
            "partitioned_metadata.partition_id as partition_id\n"
            "from raw_nuc_{0} raw, partitioned_metadata "
            "where raw.key = partitioned_metadata.{1};",
            seq_name,
            database_config_.schema.primary_key
         ));

         if (return_code->HasError()) {
            SPDLOG_ERROR(return_code->GetError());
            throw silo::PreprocessingException(return_code->GetError());
         }
      }

      for (const auto& [seq_name, reference_sequence] : reference_genomes.raw_aa_sequences) {
         silo::FastaReader fasta_reader(preprocessing_config.getGeneFilenameNoExtension(seq_name)
                                           .replace_extension(silo::preprocessing::FASTA_EXTENSION)
         );
         ZstdFastaTable::generate(
            connection, "raw_gene_" + seq_name, fasta_reader, reference_sequence
         );

         auto return_code = connection.Query(fmt::format(
            "create or replace view gene_{0} as\n"
            "select key, sequence,"
            "partitioned_metadata.partition_id as partition_id\n"
            "from raw_gene_{0} raw, partitioned_metadata "
            "where raw.key = partitioned_metadata.{1};",
            seq_name,
            database_config_.schema.primary_key
         ));
      }
   }

   preprocessing::Partitions partition_descriptor = buildPartitionDescriptorFromSQL(connection);

   SPDLOG_INFO("preprocessing - building database");

   database.build(connection, partition_descriptor, reference_genomes, order_by_clause);

   return database;
}

void Database::initializeColumn(config::ColumnType column_type, const std::string& name) {
   SPDLOG_TRACE("Initializing column {}", name);
   columns.metadata.push_back({name, column_type});
   switch (column_type) {
      case config::ColumnType::STRING:
         columns.string_columns.emplace(name, storage::column::StringColumn());
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.string_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::INDEXED_STRING: {
         auto column = storage::column::IndexedStringColumn();
         columns.indexed_string_columns.emplace(name, std::move(column));
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.indexed_string_columns.at(name).createPartition());
         }
      } break;
      case config::ColumnType::INDEXED_PANGOLINEAGE:
         columns.pango_lineage_columns.emplace(
            name, storage::column::PangoLineageColumn(alias_key)
         );
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.pango_lineage_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::DATE: {
         auto column = name == database_config.schema.date_to_sort_by
                          ? storage::column::DateColumn(true)
                          : storage::column::DateColumn(false);
         columns.date_columns.emplace(name, std::move(column));
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.date_columns.at(name).createPartition());
         }
      } break;
      case config::ColumnType::INT:
         columns.int_columns.emplace(name, storage::column::IntColumn());
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.int_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::FLOAT:
         columns.float_columns.emplace(name, storage::column::FloatColumn());
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.float_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::NUC_INSERTION:
         columns.nuc_insertion_columns.emplace(
            name, storage::column::InsertionColumn<Nucleotide>(getDefaultSequenceName<Nucleotide>())
         );
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.nuc_insertion_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::AA_INSERTION:
         columns.aa_insertion_columns.emplace(
            name, storage::column::InsertionColumn<AminoAcid>(getDefaultSequenceName<AminoAcid>())
         );
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.aa_insertion_columns.at(name).createPartition());
         }
         break;
   }
}

void Database::initializeColumns() {
   for (const auto& item : database_config.schema.metadata) {
      initializeColumn(item.getColumnType(), item.name);
   }
}

void Database::initializeNucSequences(
   const std::map<std::string, std::vector<Nucleotide::Symbol>>& reference_sequences
) {
   SPDLOG_DEBUG("build - initializing nucleotide sequences");
   for (const auto& [nuc_name, reference_sequence] : reference_sequences) {
      auto seq_store = SequenceStore<Nucleotide>(reference_sequence);
      nuc_sequences.emplace(nuc_name, std::move(seq_store));
      for (auto& partition : partitions) {
         partition.nuc_sequences.insert({nuc_name, nuc_sequences.at(nuc_name).createPartition()});
      }
   }
}

void Database::initializeAASequences(
   const std::map<std::string, std::vector<AminoAcid::Symbol>>& reference_sequences
) {
   SPDLOG_DEBUG("build - initializing amino acid sequences");
   for (const auto& [aa_name, reference_sequence] : reference_sequences) {
      auto aa_store = SequenceStore<AminoAcid>(reference_sequence);
      aa_sequences.emplace(aa_name, std::move(aa_store));
      for (auto& partition : partitions) {
         partition.aa_sequences.insert({aa_name, aa_sequences.at(aa_name).createPartition()});
      }
   }
}

void Database::finalizeInsertionIndexes() {
   tbb::parallel_for_each(partitions.begin(), partitions.end(), [](auto& partition) {
      for (auto& insertion_column : partition.columns.nuc_insertion_columns) {
         insertion_column.second.buildInsertionIndexes();
      }
      for (auto& insertion_column : partition.columns.aa_insertion_columns) {
         insertion_column.second.buildInsertionIndexes();
      }
   });
}

void Database::setDataVersion(const DataVersion& data_version) {
   SPDLOG_DEBUG("Set data version to {}", data_version.toString());
   data_version_ = data_version;
}

DataVersion Database::getDataVersion() const {
   return data_version_;
}

query_engine::QueryResult Database::executeQuery(const std::string& query) const {
   const silo::query_engine::QueryEngine query_engine(*this);

   return query_engine.executeQuery(query);
}

}  // namespace silo
