#include "silo/database.h"

#include <array>
#include <atomic>
#include <deque>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
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
#include <roaring/roaring.hh>

#include "silo/common/block_timer.h"
#include "silo/common/format_number.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/common/zstdfasta_reader.h"
#include "silo/config/database_config.h"
#include "silo/database_info.h"
#include "silo/persistence/exception.h"
#include "silo/prepare_dataset.h"
#include "silo/preprocessing/metadata.h"
#include "silo/preprocessing/metadata_validator.h"
#include "silo/preprocessing/pango_lineage_count.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/storage/aa_store.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/float_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/column_group.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"
#include "silo/storage/sequence_store.h"

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

const PangoLineageAliasLookup& Database::getAliasKey() const {
   return alias_key;
}

void Database::build(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const preprocessing::Partitions& partition_descriptor,
   const ReferenceGenomes& reference_genomes
) {
   int64_t micros = 0;
   {
      const BlockTimer timer(micros);
      for (const auto& partition : partition_descriptor.getPartitions()) {
         partitions.emplace_back(partition.getChunks());
      }
      initializeColumns();
      initializeNucSequences(reference_genomes.nucleotide_sequences);
      initializeAASequences(reference_genomes.aa_sequences);
      for (size_t partition_index = 0;
           partition_index < partition_descriptor.getPartitions().size();
           ++partition_index) {
         const auto& part = partition_descriptor.getPartitions()[partition_index];
         for (size_t chunk_index = 0; chunk_index < part.getChunks().size(); ++chunk_index) {
            const std::filesystem::path metadata_file =
               preprocessing_config.getMetadataSortedPartitionFilename(
                  partition_index, chunk_index
               );
            if (!std::filesystem::exists(metadata_file)) {
               SPDLOG_ERROR("metadata file {} not found", metadata_file.string());
               return;
            }
            SPDLOG_DEBUG("Using metadata file: {}", metadata_file.string());
            partitions[partition_index].sequence_count =
               partitions[partition_index].columns.fill(metadata_file, database_config);
         }
      }
      tbb::parallel_for(
         tbb::blocked_range<size_t>(0, partition_descriptor.getPartitions().size()),
         [&](const auto& local) {
            for (auto partition_index = local.begin(); partition_index != local.end();
                 ++partition_index) {
               const auto& part = partition_descriptor.getPartitions()[partition_index];
               for (size_t chunk_index = 0; chunk_index < part.getChunks().size(); ++chunk_index) {
                  for (const auto& [nuc_name, reference_sequence] :
                       reference_genomes.raw_nucleotide_sequences) {
                     const std::filesystem::path sequence_filename =
                        preprocessing_config.getNucSortedPartitionFilename(
                           nuc_name, partition_index, chunk_index
                        );

                     silo::ZstdFastaReader sequence_input(sequence_filename, reference_sequence);
                     SPDLOG_DEBUG("Using nucleotide sequence file: {}", sequence_filename.string());
                     partitions[partition_index].nuc_sequences.at(nuc_name).fill(sequence_input);
                  }
                  for (const auto& [aa_name, reference_sequence] :
                       reference_genomes.raw_aa_sequences) {
                     const std::filesystem::path sequence_filename =
                        preprocessing_config.getGeneSortedPartitionFilename(
                           aa_name, partition_index, chunk_index
                        );

                     silo::ZstdFastaReader sequence_input(sequence_filename, reference_sequence);
                     SPDLOG_DEBUG("Using amino acid sequence file: {}", sequence_filename.string());
                     partitions[partition_index].aa_sequences.at(aa_name).fill(sequence_input);
                  }
               }
               partitions.at(partition_index).flipBitmaps();
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
            for (const auto& bitmap : seq_store.nucleotide_symbol_n_bitmaps) {
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
   for (const auto& symbol : NUC_SYMBOLS) {
      this->size_in_bytes.at(symbol) += other.size_in_bytes.at(symbol);
   }
   return *this;
}
BitmapSizePerSymbol::BitmapSizePerSymbol() {
   for (const auto& symbol : NUC_SYMBOLS) {
      this->size_in_bytes[symbol] = 0;
   }
}

BitmapSizePerSymbol Database::calculateBitmapSizePerSymbol(const SequenceStore& seq_store) {
   BitmapSizePerSymbol global_bitmap_size_per_symbol;

   std::mutex lock;
   tbb::parallel_for_each(NUC_SYMBOLS, [&](NUCLEOTIDE_SYMBOL symbol) {
      BitmapSizePerSymbol bitmap_size_per_symbol;

      for (const SequenceStorePartition& seq_store_partition : seq_store.partitions) {
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

BitmapContainerSize Database::calculateBitmapContainerSizePerGenomeSection(
   const SequenceStore& seq_store,
   size_t section_length
) {
   const uint32_t genome_length = seq_store.reference_genome.size();

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
            for (const auto& genome_symbol : NUC_SYMBOLS) {
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
                  if (genome_symbol == NUCLEOTIDE_SYMBOL::N) {
                     bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                        .at("N")
                        .at(position_index / section_length) += statistic.n_bitset_containers;
                  } else if (genome_symbol == NUCLEOTIDE_SYMBOL::GAP) {
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
           BitmapContainerSize{seq_store.reference_genome.size(), DEFAULT_SECTION_LENGTH}}}
      );
      result.sequences.at(seq_name).bitmap_size_per_symbol =
         calculateBitmapSizePerSymbol(seq_store);
      result.sequences.at(seq_name).bitmap_container_size_per_genome_section =
         calculateBitmapContainerSizePerGenomeSection(seq_store, DEFAULT_SECTION_LENGTH);
   }
   return result;
}

std::map<std::string, std::vector<NUCLEOTIDE_SYMBOL>> Database::getNucSequences() const {
   std::map<std::string, std::vector<NUCLEOTIDE_SYMBOL>> nucleotide_sequences_map;
   for (const auto& [name, store] : nuc_sequences) {
      nucleotide_sequences_map.emplace(name, store.reference_genome);
   }
   return nucleotide_sequences_map;
}

std::map<std::string, std::vector<AA_SYMBOL>> Database::getAASequences() const {
   std::map<std::string, std::vector<AA_SYMBOL>> aa_sequences_map;
   for (const auto& [name, store] : aa_sequences) {
      aa_sequences_map.emplace(name, store.reference_sequence);
   }
   return aa_sequences_map;
}

void Database::saveDatabaseState(const std::string& save_directory) {
   const std::string database_config_filename = save_directory + "database_config.yaml";
   database_config.writeConfig(database_config_filename);

   std::ofstream alias_key_file(save_directory + "alias_key.silo");
   ::boost::archive::binary_oarchive alias_key_archive(alias_key_file);
   alias_key_archive << alias_key;

   std::ofstream partitions_file(save_directory + "partitions.silo");
   ::boost::archive::binary_oarchive partitions_archive(partitions_file);
   partitions_archive << partitions;

   std::ofstream column_file(save_directory + "column_info.silo");
   ::boost::archive::binary_oarchive column_archive(column_file);
   column_archive << columns;

   auto nuc_sequences_map = getNucSequences();
   std::ofstream nuc_sequences_file(save_directory + "nuc_sequences.silo");
   ::boost::archive::binary_oarchive nuc_sequences_archive(nuc_sequences_file);
   nuc_sequences_archive << nuc_sequences_map;

   auto aa_sequences_map = getAASequences();
   std::ofstream aa_sequences_file(save_directory + "aa_sequences.silo");
   ::boost::archive::binary_oarchive aa_sequences_archive(aa_sequences_file);
   aa_sequences_archive << aa_sequences_map;

   std::vector<std::ofstream> file_vec;
   for (uint32_t i = 0; i < partitions.size(); ++i) {
      const auto& partition_file = save_directory + "P" + std::to_string(i) + ".silo";
      file_vec.emplace_back(partition_file);

      if (!file_vec.back()) {
         throw persistence::SaveDatabaseException(
            "Cannot open partition output file " + partition_file + " for saving"
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
}

Database Database::loadDatabaseState(const std::string& save_directory) {
   Database database;
   const auto database_config_filename = save_directory + "database_config.yaml";
   database.database_config =
      silo::config::DatabaseConfigReader().readConfig(database_config_filename);

   SPDLOG_TRACE("Loading alias key from {}", save_directory + "alias_key.silo");
   std::ifstream alias_key_file(save_directory + "alias_key.silo");
   ::boost::archive::binary_iarchive alias_key_archive(alias_key_file);
   alias_key_archive >> database.alias_key;

   SPDLOG_INFO("Loading partitions from {}", save_directory);

   SPDLOG_TRACE("Loading partitions from {}", save_directory + "partitions.silo");
   std::ifstream partitions_file(save_directory + "partitions.silo");
   ::boost::archive::binary_iarchive partitions_archive(partitions_file);
   partitions_archive >> database.partitions;

   SPDLOG_TRACE("Initializing columns");
   database.initializeColumns();

   SPDLOG_TRACE("Loading column info from {}", save_directory + "column_info.silo");
   std::ifstream column_file(save_directory + "column_info.silo");
   ::boost::archive::binary_iarchive column_archive(column_file);
   column_archive >> database.columns;

   SPDLOG_TRACE("Loading nucleotide sequences from {}", save_directory + "nuc_sequences.silo");
   std::map<std::string, std::vector<NUCLEOTIDE_SYMBOL>> nuc_sequences_map;
   std::ifstream nuc_sequences_file(save_directory + "nuc_sequences.silo");
   ::boost::archive::binary_iarchive nuc_sequences_archive(nuc_sequences_file);
   nuc_sequences_archive >> nuc_sequences_map;

   SPDLOG_TRACE("Loading amino acid sequences from {}", save_directory + "aa_sequences.silo");
   std::map<std::string, std::vector<AA_SYMBOL>> aa_sequences_map;
   std::ifstream aa_sequences_file(save_directory + "aa_sequences.silo");
   ::boost::archive::binary_iarchive aa_sequences_archive(aa_sequences_file);
   aa_sequences_archive >> aa_sequences_map;

   SPDLOG_INFO("Finished loading partitions from {}", save_directory);

   database.initializeNucSequences(nuc_sequences_map);
   database.initializeAASequences(aa_sequences_map);

   SPDLOG_DEBUG("Loading partition data");
   std::vector<std::ifstream> file_vec;
   for (uint32_t i = 0; i < database.partitions.size(); ++i) {
      const auto& partition_file = save_directory + "P" + std::to_string(i) + ".silo";
      file_vec.emplace_back(partition_file);

      if (!file_vec.back()) {
         throw persistence::SaveDatabaseException(
            "Cannot open partition input file " + partition_file + " for loading"
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
   return database;
}

Database Database::preprocessing(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const config::DatabaseConfig& database_config_
) {
   Database database;
   database.database_config = database_config_;

   SPDLOG_INFO("preprocessing - validate metadata file against config");
   preprocessing::MetadataValidator().validateMedataFile(
      preprocessing_config.getMetadataInputFilename(), database_config_
   );

   SPDLOG_INFO("preprocessing - building alias key");
   database.alias_key =
      PangoLineageAliasLookup::readFromFile(preprocessing_config.getPangoLineageDefinitionFilename()
      );

   SPDLOG_INFO("preprocessing - reading reference genome");
   const ReferenceGenomes& reference_genomes =
      ReferenceGenomes::readFromFile(preprocessing_config.getReferenceGenomeFilename());

   SPDLOG_INFO("preprocessing - counting pango lineages");
   const preprocessing::PangoLineageCounts pango_descriptor(preprocessing::buildPangoLineageCounts(
      database.alias_key, preprocessing_config.getMetadataInputFilename(), database_config_
   ));

   SPDLOG_INFO("preprocessing - calculating partitions");
   const preprocessing::Partitions partition_descriptor(
      preprocessing::buildPartitions(pango_descriptor, preprocessing::Architecture::MAX_PARTITIONS)
   );

   SPDLOG_INFO("preprocessing - partitioning data");
   partitionData(
      preprocessing_config,
      partition_descriptor,
      database.alias_key,
      database_config_,
      reference_genomes
   );

   if (database_config_.schema.date_to_sort_by.has_value()) {
      SPDLOG_INFO("preprocessing - sorting chunks");
      sortChunks(
         preprocessing_config,
         partition_descriptor,
         {database_config_.schema.primary_key, database_config_.schema.date_to_sort_by.value()},
         reference_genomes
      );
   } else {
      SPDLOG_INFO("preprocessing - skipping sorting chunks because no date to sort by was specified"
      );
   }

   SPDLOG_INFO("preprocessing - building database");

   database.build(preprocessing_config, partition_descriptor, reference_genomes);

   return database;
}

void Database::initializeColumn(config::ColumnType column_type, const std::string& name) {
   SPDLOG_TRACE("Initializing column {}", name);
   switch (column_type) {
      case config::ColumnType::STRING:
         columns.string_columns.emplace(name, storage::column::StringColumn());
         for (auto& partition : partitions) {
            partition.insertColumn(name, columns.string_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::INDEXED_STRING: {
         auto column = storage::column::IndexedStringColumn();
         columns.indexed_string_columns.emplace(name, std::move(column));
         for (auto& partition : partitions) {
            partition.insertColumn(name, columns.indexed_string_columns.at(name).createPartition());
         }
      } break;
      case config::ColumnType::INDEXED_PANGOLINEAGE:
         columns.pango_lineage_columns.emplace(
            name, storage::column::PangoLineageColumn(alias_key)
         );
         for (auto& partition : partitions) {
            partition.insertColumn(name, columns.pango_lineage_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::DATE: {
         auto column = name == database_config.schema.date_to_sort_by
                          ? storage::column::DateColumn(true)
                          : storage::column::DateColumn(false);
         columns.date_columns.emplace(name, std::move(column));
         for (auto& partition : partitions) {
            partition.insertColumn(name, columns.date_columns.at(name).createPartition());
         }
      } break;
      case config::ColumnType::INT:
         columns.int_columns.emplace(name, storage::column::IntColumn());
         for (auto& partition : partitions) {
            partition.insertColumn(name, columns.int_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::FLOAT:
         columns.float_columns.emplace(name, storage::column::FloatColumn());
         for (auto& partition : partitions) {
            partition.insertColumn(name, columns.float_columns.at(name).createPartition());
         }
         break;
      case config::ColumnType::INSERTION:
         columns.insertion_columns.emplace(name, storage::column::InsertionColumn());
         for (auto& partition : partitions) {
            partition.insertColumn(name, columns.insertion_columns.at(name).createPartition());
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
   const std::map<std::string, std::vector<NUCLEOTIDE_SYMBOL>>& reference_sequences
) {
   SPDLOG_DEBUG("preprocessing - initializing nucleotide sequences");
   for (const auto& [nuc_name, reference_genome] : reference_sequences) {
      auto seq_store = SequenceStore(reference_genome);
      nuc_sequences.emplace(nuc_name, std::move(seq_store));
      for (auto& partition : partitions) {
         partition.nuc_sequences.insert({nuc_name, nuc_sequences.at(nuc_name).createPartition()});
      }
   }
}

void Database::initializeAASequences(
   const std::map<std::string, std::vector<AA_SYMBOL>>& reference_sequences
) {
   SPDLOG_DEBUG("preprocessing - initializing amino acid sequences");
   for (const auto& [aa_name, reference_genome] : reference_sequences) {
      auto aa_store = AAStore(reference_genome);
      aa_sequences.emplace(aa_name, std::move(aa_store));
      for (auto& partition : partitions) {
         partition.aa_sequences.insert({aa_name, aa_sequences.at(aa_name).createPartition()});
      }
   }
}

void Database::finalizeInsertionIndexes() {
   tbb::parallel_for_each(partitions.begin(), partitions.end(), [](auto& partition) {
      for (auto& insertion_column : partition.columns.insertion_columns) {
         insertion_column.second.buildInsertionIndex();
      }
   });
}

Database::Database() = default;

}  // namespace silo
