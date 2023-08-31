#pragma once

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
class PreprocessingConfig;
}  // namespace preprocessing
}  // namespace silo

namespace silo {

void partitionData(
   const silo::preprocessing::PreprocessingConfig& preprocessing_config,
   const preprocessing::Partitions& partitions,
   const PangoLineageAliasLookup& alias_key,
   const std::string& primary_key_field,
   const std::string& partition_by_field,
   const ReferenceGenomes& reference_genomes
);

void copyDataToPartitionDirectory(
   const preprocessing::PreprocessingConfig& preprocessing_config,
   const ReferenceGenomes& reference_genomes
);

struct SortChunkConfig {
   std::string primary_key_name;
   std::string date_column_to_sort_by;
};

void sortChunks(
   const silo::preprocessing::PreprocessingConfig& preprocessing_config,
   const preprocessing::Partitions& partitions,
   const SortChunkConfig& sort_chunk_config,
   const ReferenceGenomes& reference_genomes
);

}  // namespace silo
