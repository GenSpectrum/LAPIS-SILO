#ifndef SILO_PREPARE_DATASET_H
#define SILO_PREPARE_DATASET_H

#include <iostream>
#include <string>
#include <unordered_map>

namespace silo {

struct Partitions;
struct PangoLineageCounts;

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
   const std::unordered_map<std::string, std::string>& alias_key,
   std::istream& meta_in
);

enum Architecture { MAX_PARTITIONS, SINGLE_PARTITION, HYBRID, SINGLE_SINGLE };

Partitions buildPartitions(PangoLineageCounts pango_lineage_counts, Architecture arch);

void partitionSequences(
   const Partitions& partitions,
   std::istream& meta_in,
   std::istream& sequence_in,
   const std::string& output_prefix,
   const std::unordered_map<std::string, std::string>& alias_key,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
);

void sortChunks(
   const Partitions& partitions,
   const std::string& output_prefix,
   const std::string& metadata_file_extension,
   const std::string& sequence_file_extension
);

}  // namespace silo

#endif  // SILO_PREPARE_DATASET_H
