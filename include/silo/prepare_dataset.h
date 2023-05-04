#ifndef SILO_PREPARE_DATASET_H
#define SILO_PREPARE_DATASET_H

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>

namespace silo {

namespace preprocessing {
struct Partitions;
struct PangoLineageCounts;
class MetadataWriter;
}  // namespace preprocessing

class FastaReader;
class PangoLineageAliasLookup;

[[maybe_unused]] void pruneSequences(
   const std::filesystem::path& metadata_in,
   silo::FastaReader& sequences_in,
   std::ostream& sequences_out
);

[[maybe_unused]] void pruneMetadata(
   const std::filesystem::path& metadata_in,
   silo::FastaReader& sequences_in,
   silo::preprocessing::MetadataWriter& metadata_writer
);

void partitionSequences(
   const preprocessing::Partitions& partitions,
   const std::filesystem::path& meta_in,
   silo::FastaReader& sequence_in,
   const std::string& output_prefix,
   const PangoLineageAliasLookup& alias_key,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
);

void sortChunks(
   const preprocessing::Partitions& partitions,
   const std::string& input_prefix,
   const std::string& output_prefix,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
);

}  // namespace silo

#endif  // SILO_PREPARE_DATASET_H
