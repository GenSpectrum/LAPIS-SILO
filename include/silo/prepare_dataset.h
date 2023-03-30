#ifndef SILO_PREPARE_DATASET_H
#define SILO_PREPARE_DATASET_H

#include <iostream>
#include <string>
#include <unordered_map>

namespace silo {

namespace preprocessing {
struct Partitions;
}

struct PangoLineageCounts;
class PangoLineageAliasLookup;

[[maybe_unused]] void pruneSequences(
   std::istream& metadata_in,
   std::istream& sequences_in,
   std::ostream& sequences_out
);

[[maybe_unused]] void pruneMetadata(
   std::istream& metadata_in,
   std::istream& sequences_in,
   std::ostream& metadata_out
);

PangoLineageCounts buildPangoLineageCounts(
   const PangoLineageAliasLookup& alias_key,
   std::istream& meta_in
);

void partitionSequences(
   const preprocessing::Partitions& partitions,
   std::istream& meta_in,
   std::istream& sequence_in,
   const std::string& output_prefix,
   const PangoLineageAliasLookup& alias_key,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
);

void sortChunks(
   const preprocessing::Partitions& partitions,
   const std::string& output_prefix,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
);

}  // namespace silo

#endif  // SILO_PREPARE_DATASET_H
