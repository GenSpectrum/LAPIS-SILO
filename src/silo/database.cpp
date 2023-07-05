#include "silo/database.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <spdlog/spdlog.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for_each.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <roaring/roaring.hh>

#include "silo/common/block_timer.h"
#include "silo/common/format_number.h"
#include "silo/common/input_stream_wrapper.h"
#include "silo/common/nucleotide_symbols.h"
#include "silo/config/config_repository.h"
#include "silo/database_info.h"
#include "silo/persistence/exception.h"
#include "silo/prepare_dataset.h"
#include "silo/preprocessing/metadata_validator.h"
#include "silo/preprocessing/pango_lineage_count.h"
#include "silo/preprocessing/partition.h"
#include "silo/preprocessing/preprocessing_config.h"
#include "silo/storage/column/date_column.h"
#include "silo/storage/column/indexed_string_column.h"
#include "silo/storage/column/int_column.h"
#include "silo/storage/column/pango_lineage_column.h"
#include "silo/storage/column/string_column.h"
#include "silo/storage/database_partition.h"
#include "silo/storage/pango_lineage_alias.h"
#include "silo/storage/reference_genomes.h"

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
   const std::filesystem::path& input_folder,
   const preprocessing::Partitions& partition_descriptor
) {
   int64_t micros = 0;
   {
      BlockTimer const timer(micros);
      partitions.resize(partition_descriptor.partitions.size());
      initializeColumns();
      initializeSequences();
      for (size_t partition_index = 0; partition_index < partition_descriptor.partitions.size();
           ++partition_index) {
         const auto& part = partition_descriptor.partitions[partition_index];
         partitions[partition_index].chunks = part.chunks;
         for (size_t chunk_index = 0; chunk_index < part.chunks.size(); ++chunk_index) {
            std::filesystem::path metadata_file = input_folder;
            metadata_file += buildChunkString(partition_index, chunk_index) + ".tsv";
            if (!std::filesystem::exists(metadata_file)) {
               SPDLOG_ERROR("metadata file {} not found", metadata_file.string());
               return;
            }
            for (auto& [nuc_name, reference_sequence] : reference_genomes.nucleotide_sequences) {
               std::filesystem::path sequence_filename = input_folder;
               sequence_filename += "nuc_" + nuc_name + std::filesystem::path::preferred_separator;
               sequence_filename += buildChunkString(partition_index, chunk_index) + ".zstdfasta";

               silo::ZstdFastaReader sequence_input(sequence_filename, reference_sequence);
               SPDLOG_DEBUG("Using metadata file: {}", metadata_file.string());
               partitions[partition_index].nuc_sequences.at(nuc_name).fill(sequence_input);
            }
            partitions[partition_index].sequenceCount =
               partitions[partition_index].columns.fill(metadata_file, alias_key, database_config);
         }
      }
   }

   SPDLOG_INFO("Build took {} ms", micros);
   SPDLOG_INFO("database info: {}", getDatabaseInfo());
}

[[maybe_unused]] void Database::flipBitmaps() {
   tbb::parallel_for_each(
      partitions.begin(),
      partitions.end(),
      [&](DatabasePartition& database_partition) {
         for (auto& [_, seq_store] : database_partition.nuc_sequences) {
            auto& positions = seq_store.positions;
            tbb::parallel_for(
               tbb::blocked_range<uint32_t>(0, positions.size()),
               [&](const auto& range) {
                  for (auto position = range.begin(); position != range.end(); ++position) {
                     std::optional<NUCLEOTIDE_SYMBOL> max_symbol = std::nullopt;
                     unsigned max_count = 0;

                     for (const auto& symbol : GENOME_SYMBOLS) {
                        const unsigned count =
                           positions[position].bitmaps[static_cast<unsigned>(symbol)].cardinality();
                        if (count > max_count) {
                           max_symbol = symbol;
                           max_count = count;
                        }
                     }
                     positions[position].symbol_whose_bitmap_is_flipped = max_symbol;
                     positions[position].bitmaps[static_cast<unsigned>(max_symbol.value())].flip(
                        0, database_partition.sequenceCount
                     );
                  }
               }
            );
         }
      }
   );
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
         sequence_count += database_partition.sequenceCount;
         for (const auto& [_, seq_store] : database_partition.nuc_sequences) {
            total_size += seq_store.computeSize();
            for (const auto& bitmap : seq_store.nucleotide_symbol_n_bitmaps) {
               nucleotide_symbol_n_bitmaps_size += bitmap.getSizeInBytes(false);
            }
         }
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
   size_per_genome_symbol_and_section[genomeSymbolRepresentation(NUCLEOTIDE_SYMBOL::GAP)] =
      std::vector<size_t>((genome_length / section_length) + 1, 0);
   size_per_genome_symbol_and_section[genomeSymbolRepresentation(NUCLEOTIDE_SYMBOL::N)] =
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
   for (const auto& symbol : GENOME_SYMBOLS) {
      this->size_in_bytes.at(symbol) += other.size_in_bytes.at(symbol);
   }
   return *this;
}
BitmapSizePerSymbol::BitmapSizePerSymbol() {
   for (const auto& symbol : GENOME_SYMBOLS) {
      this->size_in_bytes[symbol] = 0;
   }
}

BitmapSizePerSymbol Database::calculateBitmapSizePerSymbol() const {
   BitmapSizePerSymbol global_bitmap_size_per_symbol;

   std::mutex lock;
   tbb::parallel_for_each(GENOME_SYMBOLS, [&](NUCLEOTIDE_SYMBOL symbol) {
      BitmapSizePerSymbol bitmap_size_per_symbol;

      for (const DatabasePartition& database_partition : partitions) {
         for (const auto& [_, seq_store] : database_partition.nuc_sequences) {
            for (const auto& position : seq_store.positions) {
               bitmap_size_per_symbol.size_in_bytes[symbol] +=
                  position.bitmaps[static_cast<unsigned>(symbol)].getSizeInBytes();
            }
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
   size_t genome_length,
   size_t section_length
) const {
   BitmapContainerSize global_bitmap_container_size_per_genome_section(
      genome_length, section_length
   );

   std::mutex lock;
   tbb::parallel_for(tbb::blocked_range<unsigned>(0U, genome_length), [&](const auto& range) {
      BitmapContainerSize bitmap_container_size_per_genome_section(genome_length, section_length);
      for (auto position_index = range.begin(); position_index != range.end(); ++position_index) {
         RoaringStatistics statistic;
         for (const auto& database_partition : partitions) {
            for (const auto& [_, seq_store] : database_partition.nuc_sequences) {
               const auto& position = seq_store.positions[position_index];
               for (const auto& genome_symbol : GENOME_SYMBOLS) {
                  const auto& bitmap = position.bitmaps[static_cast<unsigned>(genome_symbol)];

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
                           .at(genomeSymbolRepresentation(NUCLEOTIDE_SYMBOL::N))
                           .at(position_index / section_length) += statistic.n_bitset_containers;
                     } else if (genome_symbol == NUCLEOTIDE_SYMBOL::GAP) {
                        bitmap_container_size_per_genome_section.size_per_genome_symbol_and_section
                           .at(genomeSymbolRepresentation(NUCLEOTIDE_SYMBOL::GAP))
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
      }
      lock.lock();
      global_bitmap_container_size_per_genome_section += bitmap_container_size_per_genome_section;
      lock.unlock();
   });

   return global_bitmap_container_size_per_genome_section;
}

DetailedDatabaseInfo Database::detailedDatabaseInfo() const {
   constexpr uint32_t DEFAULT_SECTION_LENGTH = 500;
   BitmapSizePerSymbol const bitmap_size_per_symbol = calculateBitmapSizePerSymbol();
   BitmapContainerSize const size_per_section = calculateBitmapContainerSizePerGenomeSection(
      reference_genomes.nucleotide_sequences.begin()->second.length(), DEFAULT_SECTION_LENGTH
   );

   return DetailedDatabaseInfo{bitmap_size_per_symbol, size_per_section};
}

[[maybe_unused]] void Database::saveDatabaseState(
   const std::string& save_directory,
   const preprocessing::Partitions& partition_descriptor
) {
   {
      std::ofstream part_def_file(save_directory + "partition_descriptor.txt");
      if (!part_def_file) {
         throw persistence::SaveDatabaseException(
            "Cannot open partitioning descriptor output file " + save_directory +
            "partition_descriptor.txt"
         );
      }
      SPDLOG_INFO("Saving partitioning descriptor to {}partition_descriptor.txt", save_directory);
      partition_descriptor.save(part_def_file);
   }

   std::vector<std::ofstream> file_vec;
   for (unsigned i = 0; i < partitions.size(); ++i) {
      const auto& partition_file = save_directory + 'P' + std::to_string(i) + ".silo";
      file_vec.emplace_back(partition_file);

      if (!file_vec.back()) {
         throw persistence::SaveDatabaseException(
            "Cannot open partition output file " + partition_file + " for saving"
         );
      }
   }

   SPDLOG_INFO("Saving {} partitions...", partitions.size());

   tbb::parallel_for(static_cast<size_t>(0), partitions.size(), [&](size_t partition_index) {
      ::boost::archive::binary_oarchive output_archive(file_vec[partition_index]);
      output_archive << partitions[partition_index];
   });
   SPDLOG_INFO("Finished saving partitions", partitions.size());
}

[[maybe_unused]] void Database::loadDatabaseState(const std::string& save_directory) {
   const auto partition_descriptor_file = save_directory + "partition_descriptor.txt";
   std::ifstream part_def_file(partition_descriptor_file);
   if (!part_def_file) {
      throw persistence::LoadDatabaseException(
         "Cannot open partition_descriptor input file for loading: " + partition_descriptor_file
      );
   }
   SPDLOG_INFO("Loading partitioning definition from {}", partition_descriptor_file);

   auto partition_descriptor =
      std::make_unique<preprocessing::Partitions>(preprocessing::Partitions::load(part_def_file));

   SPDLOG_INFO("Loading partitions from {}", save_directory);
   std::vector<std::ifstream> file_vec;
   for (unsigned i = 0; i < partition_descriptor->partitions.size(); ++i) {
      const auto partition_file = save_directory + 'P' + std::to_string(i) + ".silo";
      file_vec.emplace_back(partition_file);

      if (!file_vec.back()) {
         throw persistence::LoadDatabaseException(
            "Cannot open partition input file for loading: " + partition_file
         );
      }
   }

   for (size_t partition_id = 0; partition_id < partition_descriptor->partitions.size();
        ++partition_id) {
      partitions.emplace_back();
   }
   tbb::parallel_for(
      static_cast<size_t>(0),
      partition_descriptor->partitions.size(),
      [&](size_t partition_index) {
         ::boost::archive::binary_iarchive input_archive(file_vec[partition_index]);
         input_archive >> partitions[partition_index];
      }
   );
}

void Database::preprocessing(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const config::DatabaseConfig& database_config_
) {
   database_config = database_config_;

   SPDLOG_INFO("preprocessing - validate metadata file against config");
   preprocessing::MetadataValidator().validateMedataFile(
      preprocessing_config.metadata_file, database_config_
   );

   SPDLOG_INFO("preprocessing - building alias key");
   alias_key =
      PangoLineageAliasLookup::readFromFile(preprocessing_config.pango_lineage_definition_file);

   SPDLOG_INFO("preprocessing - reading reference genome");
   reference_genomes = ReferenceGenomes::readFromFile(preprocessing_config.reference_genome_file);

   SPDLOG_INFO("preprocessing - counting pango lineages");
   const preprocessing::PangoLineageCounts pango_descriptor(preprocessing::buildPangoLineageCounts(
      alias_key, preprocessing_config.metadata_file, database_config_
   ));

   SPDLOG_INFO("preprocessing - calculating partitions");
   const preprocessing::Partitions partition_descriptor(
      preprocessing::buildPartitions(pango_descriptor, preprocessing::Architecture::MAX_PARTITIONS)
   );

   SPDLOG_INFO("preprocessing - partitioning data");
   preprocessing::MetadataReader metadata_reader(preprocessing_config.metadata_file.relative_path()
   );
   partitionData(
      partition_descriptor,
      preprocessing_config.input_directory.relative_path(),
      metadata_reader,
      preprocessing_config.partition_folder.relative_path(),
      alias_key,
      database_config_,
      reference_genomes
   );

   if (database_config_.schema.date_to_sort_by.has_value()) {
      SPDLOG_INFO("preprocessing - sorting chunks");
      sortChunks(
         partition_descriptor,
         preprocessing_config.partition_folder.relative_path(),
         preprocessing_config.sorted_partition_folder.relative_path(),
         {database_config_.schema.primary_key, database_config_.schema.date_to_sort_by.value()},
         reference_genomes
      );
   } else {
      SPDLOG_INFO("preprocessing - skipping sorting chunks because no date to sort by was specified"
      );
   }

   SPDLOG_INFO("preprocessing - building database");

   build(preprocessing_config.sorted_partition_folder.relative_path(), partition_descriptor);
}

void Database::initializeColumns() {
   for (const auto& item : database_config.schema.metadata) {
      const auto column_type = item.getColumnType();
      if (column_type == config::ColumnType::INDEXED_STRING) {
         auto column = storage::column::IndexedStringColumn();
         indexed_string_columns.emplace(item.name, std::move(column));
         for (auto& partition : partitions) {
            partition.insertColumn(
               item.name, indexed_string_columns.at(item.name).createPartition()
            );
         }
      } else if (column_type == config::ColumnType::STRING) {
         string_columns.emplace(item.name, storage::column::StringColumn());
         for (auto& partition : partitions) {
            partition.insertColumn(item.name, string_columns.at(item.name).createPartition());
         }
      } else if (column_type == config::ColumnType::INDEXED_PANGOLINEAGE) {
         pango_lineage_columns.emplace(item.name, storage::column::PangoLineageColumn());
         for (auto& partition : partitions) {
            partition.insertColumn(
               item.name, pango_lineage_columns.at(item.name).createPartition()
            );
         }
      } else if (column_type == config::ColumnType::DATE) {
         auto column = item.name == database_config.schema.date_to_sort_by
                          ? storage::column::DateColumn(true)
                          : storage::column::DateColumn(false);
         date_columns.emplace(item.name, std::move(column));
         for (auto& partition : partitions) {
            partition.insertColumn(item.name, date_columns.at(item.name).createPartition());
         }
      } else if (column_type == config::ColumnType::INT) {
         int_columns.emplace(item.name, storage::column::IntColumn());
         for (auto& partition : partitions) {
            partition.insertColumn(item.name, int_columns.at(item.name).createPartition());
         }
      } else if (column_type == config::ColumnType::FLOAT) {
         float_columns.emplace(item.name, storage::column::FloatColumn());
         for (auto& partition : partitions) {
            partition.insertColumn(item.name, float_columns.at(item.name).createPartition());
         }
      }
   }
}

void Database::initializeSequences() {
   for (const auto& [nuc_name, reference_genome] : reference_genomes.nucleotide_sequences) {
      auto seq_store = SequenceStore(reference_genome);
      nuc_sequences.emplace(nuc_name, std::move(seq_store));
      for (auto& partition : partitions) {
         partition.nuc_sequences.insert({nuc_name, nuc_sequences.at(nuc_name).createPartition()});
      }
   }
}

Database::Database() = default;

std::string buildChunkString(unsigned int partition, unsigned int chunk) {
   return "P" + std::to_string(partition) + "_C" + std::to_string(chunk);
}

}  // namespace silo
