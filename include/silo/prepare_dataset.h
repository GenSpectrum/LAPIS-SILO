#ifndef SILO_PREPARE_DATASET_H
#define SILO_PREPARE_DATASET_H

#include <filesystem>
#include <iostream>
#include <string>

namespace silo {
class FastaReader;
class PangoLineageAliasLookup;
struct ReferenceGenomes;

namespace config {
struct DatabaseConfig;
}
namespace preprocessing {
class MetadataReader;
class MetadataWriter;
struct Partitions;
}  // namespace preprocessing
}  // namespace silo

namespace silo {
[[maybe_unused]] void pruneSequences(
   silo::preprocessing::MetadataReader& metadata_reader,
   silo::FastaReader& sequences_in,
   std::ostream& sequences_out,
   const silo::config::DatabaseConfig& database_config
);

[[maybe_unused]] void pruneMetadata(
   silo::preprocessing::MetadataReader& metadata_reader,
   silo::FastaReader& sequences_in,
   silo::preprocessing::MetadataWriter& metadata_writer,
   const silo::config::DatabaseConfig& database_config
);

void partitionData(
   const preprocessing::Partitions& partitions,
   const std::filesystem::path& input_folder,
   silo::preprocessing::MetadataReader& metadata_reader,
   const std::filesystem::path& output_folder,
   const PangoLineageAliasLookup& alias_key,
   const silo::config::DatabaseConfig& database_config,
   const ReferenceGenomes& reference_genomes
);

struct SortChunkConfig {
   std::string primary_key_name;
   std::string date_column_to_sort_by;
};

void sortChunks(
   const preprocessing::Partitions& partitions,
   const std::filesystem::path& input_folder,
   const std::filesystem::path& output_folder,
   const SortChunkConfig& sort_chunk_config,
   const ReferenceGenomes& reference_genomes
);

}  // namespace silo

#endif  // SILO_PREPARE_DATASET_H
