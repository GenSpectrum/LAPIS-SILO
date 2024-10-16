#include "silo/database.h"

#include <array>
#include <atomic>
#include <cstdint>
#include <cstdlib>
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

#include <fmt/format.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_for_each.h>
#include <spdlog/spdlog.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/detail/interface_iarchive.hpp>
#include <boost/archive/detail/interface_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <duckdb.hpp>
#include <roaring/roaring.hh>

#include "silo/common/block_timer.h"
#include "silo/common/data_version.h"
#include "silo/common/format_number.h"
#include "silo/common/lineage_tree.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/panic.h"
#include "silo/config/database_config.h"
#include "silo/config/preprocessing_config.h"
#include "silo/database_info.h"
#include "silo/persistence/exception.h"
#include "silo/preprocessing/metadata_info.h"
#include "silo/preprocessing/preprocessing_exception.h"
#include "silo/query_engine/query_engine.h"
#include "silo/query_engine/query_result.h"
#include "silo/roaring/roaring_serialize.h"
#include "silo/sequence_file_reader/fasta_reader.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"
#include "silo/storage/serialize_optional.h"
#include "silo/storage/unaligned_sequence_store.h"
#include "silo/zstd/zstd_decompressor.h"
#include "silo/zstd/zstd_table.h"

namespace silo {

template <>
std::optional<std::string> Database::getDefaultSequenceName<Nucleotide>() const {
   return database_config.default_nucleotide_sequence;
}

template <>
std::optional<std::string> Database::getDefaultSequenceName<AminoAcid>() const {
   return database_config.default_amino_acid_sequence;
}

template <>
std::vector<std::string> Database::getSequenceNames<Nucleotide>() const {
   std::vector<std::string> sequence_names;
   sequence_names.reserve(nuc_sequences.size());
   for (const auto& [name, _] : nuc_sequences) {
      sequence_names.emplace_back(name);
   }
   return sequence_names;
}

template <>
std::vector<std::string> Database::getSequenceNames<AminoAcid>() const {
   std::vector<std::string> sequence_names;
   sequence_names.reserve(aa_sequences.size());
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

void Database::validate() const {
   for (const auto& partition : partitions) {
      partition.validate();
   }
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

   return DatabaseInfo{
      .sequence_count = sequence_count,
      .total_size = total_size,
      .n_bitmaps_size = nucleotide_symbol_n_bitmaps_size,
      .number_of_partitions = partitions.size()
   };
}

BitmapContainerSize::BitmapContainerSize(size_t genome_length, size_t section_length)
    : section_length(section_length),
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
      ASSERT(size_in_bytes.contains(symbol));
      size_in_bytes.at(symbol) += other.size_in_bytes.at(symbol);
   }
   return *this;
}
BitmapSizePerSymbol::BitmapSizePerSymbol() {
   for (const auto& symbol : Nucleotide::SYMBOLS) {
      size_in_bytes[symbol] = 0;
   }
}

template <typename SymbolType>
BitmapSizePerSymbol Database::calculateBitmapSizePerSymbol(
   const SequenceStore<SymbolType>& seq_store
) {
   SPDLOG_TRACE("calculateBitmapSizePerSymbol");
   BitmapSizePerSymbol global_bitmap_size_per_symbol;

   std::mutex lock;
   tbb::parallel_for_each(Nucleotide::SYMBOLS, [&](Nucleotide::Symbol symbol) {
      BitmapSizePerSymbol bitmap_size_per_symbol;

      for (const SequenceStorePartition<SymbolType>& seq_store_partition : seq_store.partitions) {
         for (const auto& position : seq_store_partition.positions) {
            ASSERT(bitmap_size_per_symbol.size_in_bytes.contains(symbol));
            bitmap_size_per_symbol.size_in_bytes[symbol] +=
               position.getBitmap(symbol)->getSizeInBytes();
         }
      }
      lock.lock();
      global_bitmap_size_per_symbol += bitmap_size_per_symbol;
      lock.unlock();
   });

   return global_bitmap_size_per_symbol;
}

namespace {

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

}  // namespace

template <typename SymbolType>
BitmapContainerSize Database::calculateBitmapContainerSizePerGenomeSection(
   const SequenceStore<SymbolType>& seq_store,
   size_t section_length
) {
   SPDLOG_TRACE("calculateBitmapContainerSizePerGenomeSection");
   const uint32_t genome_length = seq_store.reference_sequence.size();

   BitmapContainerSize bitmap_container_size_per_genome_section(genome_length, section_length);

   for (size_t position_idx = 0; position_idx < genome_length; position_idx++) {
      RoaringStatistics statistic;
      for (const auto& seq_store_partition : seq_store.partitions) {
         const auto& position = seq_store_partition.positions[position_idx];
         for (const auto& genome_symbol : Nucleotide::SYMBOLS) {
            const auto& bitmap = *position.getBitmap(genome_symbol);

            roaring_bitmap_statistics(&bitmap.roaring, &statistic);
            addStatisticToBitmapContainerSize(
               statistic, bitmap_container_size_per_genome_section.bitmap_container_size_statistic
            );

            bitmap_container_size_per_genome_section.total_bitmap_size_computed +=
               bitmap.getSizeInBytes();
            bitmap_container_size_per_genome_section.total_bitmap_size_frozen +=
               bitmap.getFrozenSizeInBytes();

            if (statistic.n_bitset_containers > 0) {
               if (genome_symbol == Nucleotide::SYMBOL_MISSING) {
                  bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                     .at("N")
                     .at(position_idx / section_length) += statistic.n_bitset_containers;
               } else if (genome_symbol == Nucleotide::Symbol::GAP) {
                  bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                     .at("GAP")
                     .at(position_idx / section_length) += statistic.n_bitset_containers;
               } else {
                  bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                     .at("NOT_N_NOT_GAP")
                     .at(position_idx / section_length) += statistic.n_bitset_containers;
               }
            }
         }
      }
   }

   return bitmap_container_size_per_genome_section;
}

DetailedDatabaseInfo Database::detailedDatabaseInfo() const {
   SPDLOG_TRACE("detailedDatabaseInfo");
   constexpr uint32_t DEFAULT_SECTION_LENGTH = 500;
   DetailedDatabaseInfo result;
   for (const auto& [seq_name, seq_store] : nuc_sequences) {
      result.sequences.insert(
         {seq_name,
          SequenceStoreStatistics{
             .bitmap_size_per_symbol = BitmapSizePerSymbol(),
             .bitmap_container_size_per_genome_section =
                BitmapContainerSize{seq_store.reference_sequence.size(), DEFAULT_SECTION_LENGTH}
          }}
      );
      result.sequences.at(seq_name).bitmap_size_per_symbol =
         calculateBitmapSizePerSymbol(seq_store);
      result.sequences.at(seq_name).bitmap_container_size_per_genome_section =
         calculateBitmapContainerSizePerGenomeSection(seq_store, DEFAULT_SECTION_LENGTH);
   }
   return result;
}

std::vector<std::vector<Nucleotide::Symbol>> Database::getNucSequences() const {
   std::vector<std::vector<Nucleotide::Symbol>> nuc_reference_sequences;
   nuc_reference_sequences.reserve(nuc_sequence_names.size());
   for (const auto& name : nuc_sequence_names) {
      nuc_reference_sequences.emplace_back(nuc_sequences.at(name).reference_sequence);
   }
   return nuc_reference_sequences;
}

std::vector<std::vector<AminoAcid::Symbol>> Database::getAASequences() const {
   std::vector<std::vector<AminoAcid::Symbol>> aa_reference_sequences;
   aa_reference_sequences.reserve(aa_sequence_names.size());
   for (const auto& name : aa_sequence_names) {
      aa_reference_sequences.emplace_back(this->aa_sequences.at(name).reference_sequence);
   }
   return aa_reference_sequences;
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

void Database::saveDatabaseState(const std::filesystem::path& save_directory) {
   if (getDataVersionTimestamp().value.empty()) {
      throw persistence::SaveDatabaseException(
         "Corrupted database (Data version is empty). Cannot save database."
      );
   }

   std::filesystem::create_directory(save_directory);
   if (!std::filesystem::exists(save_directory)) {
      auto error = fmt::format(
         "Could not create the directory '{}' which contains the saved databases outputs",
         save_directory.string()
      );
      SPDLOG_ERROR(error);
      throw persistence::SaveDatabaseException(error);
   }

   const std::filesystem::path versioned_save_directory =
      save_directory / getDataVersionTimestamp().value;
   SPDLOG_INFO("Saving database to '{}'", versioned_save_directory.string());

   if (std::filesystem::exists(versioned_save_directory)) {
      auto error = fmt::format(
         "In the output directory {} there already exists a file/folder with the name equal to "
         "the current data-version: {}",
         save_directory.string(),
         getDataVersionTimestamp().value
      );
      SPDLOG_ERROR(error);
      throw persistence::SaveDatabaseException(error);
   }

   std::filesystem::create_directory(versioned_save_directory);

   SPDLOG_INFO("Saving database config and schema");

   const std::filesystem::path database_config_filename =
      versioned_save_directory / "database_config.yaml";
   database_config.writeConfig(database_config_filename);

   std::ofstream lineage_file(versioned_save_directory / "lineage_definitions.silo");
   boost::archive::binary_oarchive lineage_archive(lineage_file);
   lineage_archive << lineage_tree;

   std::ofstream partitions_file =
      openOutputFileOrThrow(versioned_save_directory / "partitions.silo");
   ::boost::archive::binary_oarchive partitions_archive(partitions_file);
   partitions_archive << partitions;

   std::ofstream column_file = openOutputFileOrThrow(versioned_save_directory / "column_info.silo");
   ::boost::archive::binary_oarchive column_archive(column_file);
   column_archive << columns;

   SPDLOG_INFO("Saving database sequence schema");

   const std::vector<std::vector<Nucleotide::Symbol>> nuc_reference_sequences = getNucSequences();
   std::ofstream nuc_sequences_file =
      openOutputFileOrThrow(versioned_save_directory / "nuc_sequences.silo");
   ::boost::archive::binary_oarchive nuc_sequences_archive(nuc_sequences_file);
   nuc_sequences_archive << nuc_sequence_names;
   nuc_sequences_archive << nuc_reference_sequences;

   auto aa_sequences_map = getAASequences();
   std::ofstream aa_sequences_file =
      openOutputFileOrThrow(versioned_save_directory / "aa_sequences.silo");
   ::boost::archive::binary_oarchive aa_sequences_archive(aa_sequences_file);
   aa_sequences_archive << aa_sequence_names;
   aa_sequences_archive << aa_sequences_map;

   SPDLOG_INFO("Saving unaligned sequence data");
   for (size_t sequence_idx = 0; sequence_idx < nuc_sequences.size(); ++sequence_idx) {
      const auto sequence_name = nuc_sequence_names.at(sequence_idx);
      const auto& sequence_store = unaligned_nuc_sequences.at(sequence_name);
      const std::filesystem::path unaligned_sequence_directory =
         versioned_save_directory / fmt::format("unaligned_nuc_{}", sequence_idx);
      SPDLOG_DEBUG(
         "Saving unaligned sequence {} ({}) to folder '{}'",
         sequence_idx,
         sequence_name,
         unaligned_sequence_directory.string()
      );
      sequence_store.saveFolder(unaligned_sequence_directory);
   }

   std::vector<std::ofstream> partition_archives;
   for (uint32_t i = 0; i < partitions.size(); ++i) {
      const auto& partition_archive =
         versioned_save_directory / ("P" + std::to_string(i) + ".silo");
      partition_archives.emplace_back(openOutputFileOrThrow(partition_archive));

      if (!partition_archives.back()) {
         throw persistence::SaveDatabaseException(
            "Cannot open partition output file " + partition_archive.string() + " for saving"
         );
      }
   }

   SPDLOG_INFO("Saving {} partitions...", partitions.size());
   tbb::parallel_for(tbb::blocked_range<size_t>(0, partitions.size()), [&](const auto& local) {
      for (size_t partition_index = local.begin(); partition_index != local.end();
           partition_index++) {
         ::boost::archive::binary_oarchive output_archive(partition_archives[partition_index]);
         partitions[partition_index].serializeData(output_archive, 0);
      }
   });
   SPDLOG_INFO("Finished saving partitions", partitions.size());

   std::ofstream data_version_file =
      openOutputFileOrThrow(versioned_save_directory / "data_version.silo");
   data_version_.saveToFile(data_version_file);
}

namespace {
DataVersion loadDataVersion(const std::filesystem::path& filename) {
   if (!std::filesystem::is_regular_file(filename)) {
      auto error = fmt::format("Input file {} could not be opened.", filename.string());
      throw persistence::LoadDatabaseException(error);
   }
   auto data_version = DataVersion::fromFile(filename);
   if (data_version == std::nullopt) {
      auto error_message = fmt::format(
         "Data version file {} did not contain a valid data version", filename.string()
      );
      SPDLOG_ERROR(error_message);
      throw persistence::LoadDatabaseException(error_message);
   }
   return data_version.value();
}
}  // namespace

Database Database::loadDatabaseState(const std::filesystem::path& save_directory) {
   Database database;
   const auto database_config_filename = save_directory / "database_config.yaml";
   database.database_config =
      silo::config::DatabaseConfigReader().readConfig(database_config_filename);
   database.unaligned_sequences_directory = save_directory;

   SPDLOG_TRACE(
      "Loading lineage definitions from {}", (save_directory / "lineage_definitions.silo").string()
   );
   std::ifstream lineage_file = openInputFileOrThrow(save_directory / "lineage_definitions.silo");
   boost::archive::binary_iarchive lineage_archive(lineage_file);
   lineage_archive >> database.lineage_tree;

   SPDLOG_TRACE("Loading partitions from {}", (save_directory / "partitions.silo").string());
   std::ifstream partitions_file = openInputFileOrThrow(save_directory / "partitions.silo");
   boost::archive::binary_iarchive partitions_archive(partitions_file);
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
   std::vector<std::string> nuc_sequence_names;
   std::vector<std::vector<Nucleotide::Symbol>> nuc_sequences;
   std::ifstream nuc_sequences_file = openInputFileOrThrow(save_directory / "nuc_sequences.silo");
   ::boost::archive::binary_iarchive nuc_sequences_archive(nuc_sequences_file);
   nuc_sequences_archive >> nuc_sequence_names;
   nuc_sequences_archive >> nuc_sequences;

   SPDLOG_TRACE(
      "Loading amino acid sequences from {}", (save_directory / "aa_sequences.silo").string()
   );
   std::vector<std::string> aa_sequence_names;
   std::vector<std::vector<AminoAcid::Symbol>> aa_sequences;
   std::ifstream aa_sequences_file = openInputFileOrThrow(save_directory / "aa_sequences.silo");
   ::boost::archive::binary_iarchive aa_sequences_archive(aa_sequences_file);
   aa_sequences_archive >> aa_sequence_names;
   aa_sequences_archive >> aa_sequences;

   SPDLOG_INFO(
      "Finished loading partitions from {}", (save_directory / "aa_sequences.silo").string()
   );

   database.initializeNucSequences(nuc_sequence_names, nuc_sequences);
   database.initializeAASequences(aa_sequence_names, aa_sequences);

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
   SPDLOG_INFO("Database info after loading: {}", database.getDatabaseInfo());

   return database;
}

void Database::initializeColumn(const config::DatabaseMetadata& metadata) {
   const std::string& name = metadata.name;
   const config::ColumnType column_type = metadata.getColumnType();
   SPDLOG_TRACE("Initializing column {}", name);
   columns.metadata.push_back({name, column_type});
   switch (column_type) {
      case config::ColumnType::STRING:
         columns.string_columns.emplace(name, storage::column::StringColumn());
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.string_columns.at(name).createPartition());
         }
         return;
      case config::ColumnType::INDEXED_STRING: {
         if (metadata.generate_lineage_index) {
            auto column = storage::column::IndexedStringColumn(lineage_tree);
            columns.indexed_string_columns.emplace(name, std::move(column));
         } else {
            auto column = storage::column::IndexedStringColumn();
            columns.indexed_string_columns.emplace(name, std::move(column));
         }
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.indexed_string_columns.at(name).createPartition());
         }
      }
         return;
      case config::ColumnType::DATE: {
         auto column = name == database_config.schema.date_to_sort_by
                          ? storage::column::DateColumn(true)
                          : storage::column::DateColumn(false);
         columns.date_columns.emplace(name, std::move(column));
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.date_columns.at(name).createPartition());
         }
      }
         return;
      case config::ColumnType::BOOL:
         columns.bool_columns.emplace(name, storage::column::BoolColumn());
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.bool_columns.at(name).createPartition());
         }
         return;
      case config::ColumnType::INT:
         columns.int_columns.emplace(name, storage::column::IntColumn());
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.int_columns.at(name).createPartition());
         }
         return;
      case config::ColumnType::FLOAT:
         columns.float_columns.emplace(name, storage::column::FloatColumn());
         for (auto& partition : partitions) {
            partition.columns.metadata.push_back({name, column_type});
            partition.insertColumn(name, columns.float_columns.at(name).createPartition());
         }
         return;
   }
   UNREACHABLE();
}

void Database::initializeColumns() {
   for (const auto& item : database_config.schema.metadata) {
      initializeColumn(item);
   }
}

void Database::initializeNucSequences(
   const std::vector<std::string>& sequence_names,
   const std::vector<std::vector<Nucleotide::Symbol>>& reference_sequences
) {
   nuc_sequence_names = sequence_names;

   SPDLOG_DEBUG("build - initializing nucleotide sequences");
   SPDLOG_TRACE("initializing aligned nucleotide sequences");
   for (size_t sequence_idx = 0; sequence_idx < sequence_names.size(); ++sequence_idx) {
      const auto& sequence_name = sequence_names.at(sequence_idx);
      const auto& reference_sequence = reference_sequences.at(sequence_idx);
      auto seq_store = SequenceStore<Nucleotide>(reference_sequence);
      nuc_sequences.emplace(sequence_name, std::move(seq_store));
      for (auto& partition : partitions) {
         partition.nuc_sequences.insert(
            {sequence_name, nuc_sequences.at(sequence_name).createPartition()}
         );
      }
   }
   SPDLOG_TRACE("initializing unaligned nucleotide sequences");
   for (size_t sequence_idx = 0; sequence_idx < sequence_names.size(); ++sequence_idx) {
      const auto& sequence_name = sequence_names.at(sequence_idx);
      const auto& reference_sequence = reference_sequences.at(sequence_idx);
      const std::filesystem::path sequence_directory =
         unaligned_sequences_directory / fmt::format("unaligned_nuc_{}", sequence_idx);
      std::filesystem::create_directory(sequence_directory);
      if (!std::filesystem::is_directory(sequence_directory)) {
         SPDLOG_TRACE(
            "Sequence directory for unaligned sequences {} could not be created.",
            sequence_directory.string()
         );
      }
      auto seq_store = silo::UnalignedSequenceStore(
         sequence_directory, ReferenceGenomes::vectorToString<Nucleotide>(reference_sequence)
      );
      unaligned_nuc_sequences.emplace(sequence_name, std::move(seq_store));
      for (auto& partition : partitions) {
         partition.unaligned_nuc_sequences.insert(
            {sequence_name, unaligned_nuc_sequences.at(sequence_name).createPartition()}
         );
      }
   }
}

void Database::initializeAASequences(
   const std::vector<std::string>& sequence_names,
   const std::vector<std::vector<AminoAcid::Symbol>>& reference_sequences
) {
   aa_sequence_names = sequence_names;

   SPDLOG_DEBUG("build - initializing amino acid sequences");
   SPDLOG_TRACE("initializing aligned amino acid sequences");
   for (size_t sequence_idx = 0; sequence_idx < sequence_names.size(); ++sequence_idx) {
      const auto& aa_sequence_name = aa_sequence_names.at(sequence_idx);
      const auto& reference_sequence = reference_sequences.at(sequence_idx);
      auto aa_store = SequenceStore<AminoAcid>(reference_sequence);
      aa_sequences.emplace(aa_sequence_name, std::move(aa_store));
      for (auto& partition : partitions) {
         partition.aa_sequences.insert(
            {aa_sequence_name, aa_sequences.at(aa_sequence_name).createPartition()}
         );
      }
   }
}

void Database::setDataVersion(const DataVersion& data_version) {
   SPDLOG_DEBUG("Set data version to {}", data_version.toString());
   data_version_ = data_version;
}

DataVersion::Timestamp Database::getDataVersionTimestamp() const {
   return data_version_.timestamp;
}

query_engine::QueryResult Database::executeQuery(const std::string& query) const {
   const silo::query_engine::QueryEngine query_engine(*this);

   return query_engine.executeQuery(query);
}

}  // namespace silo
